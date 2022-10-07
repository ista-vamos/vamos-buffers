#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "shmbuf/buffer.h"
#include "stream.h"

/*****
 * STREAM
 *****/

const char *shm_stream_get_name(shm_stream *stream) {
    assert(stream);
    return stream->name;
}

const char *shm_stream_get_type(shm_stream *stream) {
    assert(stream);
    return stream->type;
}

static uint64_t last_stream_id = 0;

void shm_stream_init(shm_stream *stream, struct buffer *incoming_events_buffer,
                     size_t event_size, shm_stream_is_ready_fn is_ready,
                     shm_stream_filter_fn filter, shm_stream_alter_fn alter,
                     shm_stream_destroy_fn destroy,
                     const char *const type, const char *const name) {
    stream->id = ++last_stream_id;
    stream->event_size = event_size;
    stream->incoming_events_buffer = incoming_events_buffer;
    /* TODO: maybe we could just have a boolean flag that would be set
     * by a particular stream implementation instead of this function call?
     * Or a special universal event that is sent by the source...*/
    stream->is_ready = is_ready;
    stream->filter = filter;
    stream->alter = alter;
    stream->destroy = destroy;
    stream->type = type;
    stream->name = strdup(name);
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
}

size_t shm_stream_id(shm_stream *stream) {
    return stream->id;
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

struct event_record *shm_stream_get_event_record(shm_stream *stream, shm_kind kind) {
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
        struct event_record *recs
                = buffer_get_avail_events(stream->incoming_events_buffer, &sz);
        size_t max_kind = get_max_kind(recs, sz);
        size_t cache_sz = (max_kind > MAX_EVENTS_CACHE_SIZE ? MAX_EVENTS_CACHE_SIZE : max_kind) + 1;
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

struct event_record *shm_stream_get_event_record_no_cache(shm_stream *stream, shm_kind kind) {
        size_t sz;
        struct event_record *recs
                = buffer_get_avail_events(stream->incoming_events_buffer, &sz);
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
void shm_stream_get_dropped_event(shm_stream *stream,
                                  shm_event_dropped *dropped_ev, size_t id,
                                  uint64_t n) {
    dropped_ev->base.id = id;
    dropped_ev->base.kind = shm_get_dropped_kind();
    dropped_ev->n = n;
#ifdef DUMP_STATS
    stream->dropped_events += n;
#else
    (void)stream;
#endif
}

bool shm_stream_is_ready(shm_stream *s) {
    return s->is_ready(s);
}

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

size_t shm_stream_event_size(shm_stream *s) {
    return s->event_size;
}

int stream_register_event(shm_stream *stream, const char *name, size_t kind) {
    return buffer_register_event(stream->incoming_events_buffer, name, kind);
}

int stream_register_events(shm_stream *stream, size_t ev_nums, ...) {
    va_list ap;
    va_start(ap, ev_nums);
    return buffer_register_events(stream->incoming_events_buffer, ev_nums, ap);
}

int stream_register_all_events(shm_stream *stream) {
    return buffer_register_all_events(stream->incoming_events_buffer);
}


void shm_stream_notify_last_processed_id(shm_stream *stream, shm_eventid id) {
    buffer_set_last_processed_id(stream->incoming_events_buffer, id);
}

void shm_stream_notify_dropped(shm_stream *stream, uint64_t begin_id,
                               uint64_t end_id) {
    buffer_notify_dropped(stream->incoming_events_buffer, begin_id, end_id);
}

void shm_stream_detach(shm_stream *stream) {
    buffer_set_attached(stream->incoming_events_buffer, false);
}

void shm_stream_destroy(shm_stream *stream) {
    shm_stream_detach(stream);

    free(stream->name);
    free(stream->events_cache);

    if (stream->destroy) {
        stream->destroy(stream);
    }
}
