#include "vamos-buffers/core/stream.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "vamos-buffers/core/vector-macro.h"
#include "vamos-buffers/shmbuf/buffer.h"
#include "vamos-buffers/core/utils.h"

/*****
 * STREAM
 *****/

/* FIXME: we duplicate the counter in stream here. Maybe rather set the funs to
 * NULL? */
static void default_hole_init(shm_event *ev) {
    ev->kind = shm_get_hole_kind();
    ((shm_event_default_hole *)ev)->n = 0;
}

static void default_hole_update(shm_event *hole, shm_event *ev) {
    (void)ev;
    ++((shm_event_default_hole *)hole)->n;
}

static shm_stream_hole_handling default_hole_handling = {
    .hole_event_size = sizeof(shm_event_default_hole),
    .init = default_hole_init,
    .update = default_hole_update};

static uint64_t last_stream_id = 0;

void shm_stream_init(shm_stream *stream, struct buffer *incoming_events_buffer,
                     size_t event_size, shm_stream_is_ready_fn is_ready,
                     shm_stream_filter_fn filter, shm_stream_alter_fn alter,
                     shm_stream_destroy_fn destroy,
                     const shm_stream_hole_handling *hole_handling,
                     const char *const type, const char *const name) {
    stream->id = ++last_stream_id;
    stream->event_size = event_size;
    stream->incoming_events_buffer = incoming_events_buffer;
    stream->substreams_no = 0;
    /* TODO: maybe we could just have a boolean flag that would be set
     * by a particular stream implementation instead of this function call?
     * Or a special universal event that is sent by the source...*/
    stream->is_ready = is_ready;
    stream->filter = filter;
    stream->alter = alter;
    stream->destroy = destroy;
    stream->type = xstrdup(type);
    stream->name = xstrdup(name);
#ifndef NDEBUG
    stream->last_event_id = 0;
#endif
    stream->events_cache = NULL;
#ifdef DUMP_STATS
    stream->read_events = 0;
    stream->fetched_events = 0;
    stream->consumed_events = 0;
    stream->dropped_events = 0;
    stream->slept_waiting_for_ev = 0;
#endif

    hole_handling = hole_handling ? hole_handling : &default_hole_handling;
    assert(hole_handling->hole_event_size > 0 && hole_handling->update &&
           hole_handling->init);

    stream->hole_handling = *hole_handling;
    stream->parent_stream = NULL;
    VEC_INIT(stream->substreams);
}

static void shm_substream_destroy(shm_stream *stream) {
    shm_stream_detach(stream);

    for (unsigned i = 0; i < VEC_SIZE(stream->substreams); ++i) {
        shm_stream *sub = stream->substreams[i];
        if (shm_stream_is_ready(sub)) {
            fprintf(stderr,
                    "warn: destroying substream %lu of %s (%lu) which is still "
                    "ready\n",
                    shm_stream_id(sub), stream->name, shm_stream_id(stream));
        }
        shm_substream_destroy(sub);
    }

    if (stream->destroy) {
        stream->destroy(stream);
    }

    free(stream->type);
    free(stream->name);
    free(stream->events_cache);

    release_shared_sub_buffer(stream->incoming_events_buffer);
    free(stream);
}

void shm_stream_destroy(shm_stream *stream) {
    shm_stream_detach(stream);

    for (unsigned i = 0; i < VEC_SIZE(stream->substreams); ++i) {
        shm_stream *sub = stream->substreams[i];
        if (shm_stream_is_ready(sub)) {
            fprintf(stderr,
                    "warn: destroying substream %lu of %s (%lu) which is still "
                    "ready\n",
                    shm_stream_id(sub), stream->name, shm_stream_id(stream));
        }
        shm_substream_destroy(sub);
    }

    if (stream->destroy) {
        stream->destroy(stream);
    }

    free(stream->type);
    free(stream->name);
    free(stream->events_cache);

    release_shared_buffer(stream->incoming_events_buffer);
    free(stream);
}

size_t shm_stream_id(shm_stream *stream) { return stream->id; }

const char *shm_stream_get_name(shm_stream *stream) {
    assert(stream);
    return stream->name;
}

const char *shm_stream_get_type(shm_stream *stream) {
    assert(stream);
    return stream->type;
}

_Bool shm_stream_has_new_substreams(shm_stream *stream) {
    return buffer_get_sub_buffers_no(stream->incoming_events_buffer) >
           stream->substreams_no;
}

_Bool shm_stream_is_substream(shm_stream *stream) {
    return stream->parent_stream != NULL;
}

shm_stream *shm_stream_create_substream(
    shm_stream *stream, shm_stream_is_ready_fn is_ready,
    shm_stream_filter_fn filter, shm_stream_alter_fn alter,
    shm_stream_destroy_fn destroy, shm_stream_hole_handling *hole_handling) {
    if (!shm_stream_has_new_substreams(stream)) {
        return NULL;
    }
    size_t substream_no = ++stream->substreams_no;
    char *key = get_sub_buffer_key(
        buffer_get_key(stream->incoming_events_buffer), substream_no);
    struct buffer *shmbuffer = get_shared_buffer(key);
    assert(shmbuffer && "Getting the shm buffer failed");

    shm_stream *substream = xalloc(sizeof *substream);

    size_t elem_size = buffer_elem_size(shmbuffer);
    assert(elem_size > 0);

    char *substream_name =
        get_sub_buffer_key(shm_stream_get_name(stream), substream_no);

    shm_stream_init(
        substream, shmbuffer, elem_size, is_ready ? is_ready : stream->is_ready,
        filter ? filter : stream->filter, alter ? alter : stream->alter,
        destroy ? destroy : stream->destroy,
        hole_handling ? hole_handling : &stream->hole_handling,
        shm_stream_get_type(stream), substream_name);
    substream->parent_stream = stream;
    free(substream_name);

    VEC_PUSH(stream->substreams, &substream);

    return substream;
}

struct event_record *shm_stream_get_avail_events(shm_stream *s, size_t *sz) {
    return buffer_get_avail_events(s->incoming_events_buffer, sz);
}

#define MAX_EVENTS_CACHE_SIZE 128

static shm_kind get_max_kind(struct event_record *recs, size_t size) {
    shm_kind ret = 0;
    for (size_t i = 0; i < size; ++i) {
        if (recs[i].kind > ret)
            ret = recs[i].kind;
    }

    return ret;
}

struct event_record *shm_stream_get_event_record(shm_stream *stream,
                                                 shm_kind kind) {
    assert(kind > 0 && "Got invalid kind");
    struct event_record *rec = NULL;
    if (stream->events_cache) {
        if (kind < MAX_EVENTS_CACHE_SIZE) {
            rec = &stream->events_cache[kind];
            assert(rec->kind == kind);
            return rec;
        } else {
            return shm_stream_get_event_record_no_cache(stream, kind);
        }
    } else {
        /* create cache */
        size_t sz;
        struct event_record *recs =
            buffer_get_avail_events(stream->incoming_events_buffer, &sz);
        size_t max_kind = get_max_kind(recs, sz);
        size_t cache_sz =
            (max_kind > MAX_EVENTS_CACHE_SIZE ? MAX_EVENTS_CACHE_SIZE
                                              : max_kind) +
            1;
        assert(cache_sz > 0 && "Invalid cache size");
        stream->events_cache = malloc(cache_sz * sizeof(struct event_record));
        /* cache elements that fit into the cache (cache is indexed by kinds) */
        for (size_t i = 0; i < sz; ++i) {
            if (recs[i].kind < cache_sz) {
                stream->events_cache[recs[i].kind] = recs[i];
            }
            if (recs[i].kind == kind)
                rec = &recs[i];
        }
        return rec;
    }
}

struct event_record *shm_stream_get_event_record_no_cache(shm_stream *stream,
                                                          shm_kind kind) {
    size_t sz;
    struct event_record *recs =
        buffer_get_avail_events(stream->incoming_events_buffer, &sz);
    for (size_t i = 0; i < sz; ++i) {
        if (recs[i].kind == kind)
            return &recs[i];
    }
    return NULL;
}

/* the number of elements in the (shared memory) buffer of the stream */
size_t shm_stream_buffer_size(shm_stream *s) {
    return buffer_size(s->incoming_events_buffer);
}

/* the capacity the (shared memory) buffer of the stream */
size_t shm_stream_buffer_capacity(shm_stream *s) {
    return buffer_capacity(s->incoming_events_buffer);
}

/* FIXME: no longer related to stream */
void shm_stream_prepare_hole_event(shm_stream *stream, shm_event *hole_event,
                                   size_t id, uint64_t n) {
    assert(hole_event->kind > 0 && "init fun set wrong kind");
    hole_event->id = id;
#ifdef DUMP_STATS
    stream->dropped_events += n;
#else
    (void)stream;
    (void)n;
#endif
}

bool shm_stream_is_ready(shm_stream *s) { return s->is_ready(s); }

bool shm_stream_is_finished(shm_stream *s) {
    return shm_stream_buffer_size(s) == 0 && !shm_stream_is_ready(s);
}

void *shm_stream_read_events(shm_stream *s, size_t *num) {
    /* the buffer may be already destroyed on the client's side,
     * but still may have some events to read */
    /* assert(shm_stream_is_ready(s)); */
    return buffer_read_pointer(s->incoming_events_buffer, num);
}

bool shm_stream_consume(shm_stream *stream, size_t num) {
#ifdef DUMP_STATS
    stream->consumed_events += num;
#endif
    return buffer_drop_k(stream->incoming_events_buffer, num);
}

const char *shm_stream_get_str(shm_stream *stream, uint64_t elem) {
    return buffer_get_str(stream->incoming_events_buffer, elem);
}

size_t shm_stream_event_size(shm_stream *s) { return s->event_size; }

int shm_stream_register_event(shm_stream *stream, const char *name,
                              size_t kind) {
    return buffer_register_event(stream->incoming_events_buffer, name, kind);
}

int shm_stream_register_events(shm_stream *stream, size_t ev_nums, ...) {
    va_list ap;
    va_start(ap, ev_nums);
    int ret =
        buffer_register_events(stream->incoming_events_buffer, ev_nums, ap);
    va_end(ap);
    return ret;
}

int shm_stream_register_all_events(shm_stream *stream) {
    return buffer_register_all_events(stream->incoming_events_buffer);
}

void shm_stream_notify_last_processed_id(shm_stream *stream, shm_eventid id) {
    buffer_set_last_processed_id(stream->incoming_events_buffer, id);
}

void shm_stream_notify_dropped(shm_stream *stream, uint64_t begin_id,
                               uint64_t end_id) {
    buffer_notify_dropped(stream->incoming_events_buffer, begin_id, end_id);
}

void shm_stream_attach(shm_stream *stream) {
    buffer_set_attached(stream->incoming_events_buffer, true);
}

void shm_stream_detach(shm_stream *stream) {
    buffer_set_attached(stream->incoming_events_buffer, false);
}

void shm_stream_dump_events(shm_stream *stream) {
    size_t evs_num;
    struct event_record *events =
        buffer_get_avail_events(stream->incoming_events_buffer, &evs_num);
    for (size_t i = 0; i < evs_num; ++i) {
        fprintf(stderr,
                "[%s:%s] event: %-20s, kind: %-3lu, size: %-3lu, sig: %s\n",
                stream->name, stream->type, events[i].name, events[i].kind,
                events[i].size, events[i].signature);
    }
}
