#include "vamos-buffers/core/shamon.h"

#include <assert.h>
#include <immintrin.h> /* _mm_pause */
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

#include "vamos-buffers/core/arbiter.h"
#include "vamos-buffers/core/par_queue.h"
#include "vamos-buffers/core/stream.h"
#include "vamos-buffers/core/utils.h"
#include "vamos-buffers/core/vector-aligned.h"
#include "vamos-buffers/core/vector-macro.h"

typedef struct _shamon {
    VEC(streams, vms_stream *);
    /* vms_arbiter_buffers stored in this vector assume
     * they are aligned in memory, so use aligned vector */
    vms_vector_aligned buffers;
    VEC(buffer_threads, thrd_t);
    /* callbacks and their data */
    shamon_process_events_fn process_events;
    void *process_events_data;
    /* the memory for passing the next event in the default
       process_events handler */
    vms_event *_ev;
    size_t _ev_size;
} shamon;

#define _buffers(shmn) ((vms_vector *)&shmn->buffers)

#define SLEEP_NS_INIT (50)
#define SLEEP_THRESHOLD_NS (10000000)

static int default_buffer_manager_thrd(void *data) {
    vms_arbiter_buffer *buffer = (vms_arbiter_buffer *)data;
    vms_stream *stream = vms_arbiter_buffer_stream(buffer);
    register vms_stream_alter_fn alter = stream->alter;
    register vms_stream_filter_fn filter = stream->filter;

    // wait for buffer->active
    while (!vms_arbiter_buffer_active(buffer)) _mm_pause();

    printf("Running fetch & autodrop for stream %s\n",
           vms_stream_get_name(stream));

    void *ev, *out;
    while (1) {
        ev = vms_stream_fetch_dropping(stream, buffer);
        if (!ev) {
            break;
        }
        if (filter && !filter(stream, ev)) {
            vms_stream_consume(stream, 1);
            continue;
        }

        out = vms_arbiter_buffer_write_ptr(buffer);
        assert(out && "No space in the buffer");
        alter(stream, ev, out);
        vms_arbiter_buffer_write_finish(buffer);
        vms_stream_consume(stream, 1);
    }

    // TODO: we should check if the stream is finished and remove it
    // in that case
    printf("BMM for stream %lu (%s) exits\n", stream->id,
           vms_stream_get_type(stream));
    thrd_exit(EXIT_SUCCESS);
}

static vms_event *default_process_events(vms_vector *buffers, void *data,
                                         vms_stream **streamret) {
    assert(buffers);
    assert(data);
    shamon *shmn = (shamon *)data;
    vms_stream *stream;

    // use static counter to do round robin -- so that some stream
    // does not starve
    static unsigned i = 0;
    vms_arbiter_buffer *buffer = NULL;
    size_t qsize;

    // reset counter if we're at the end
    if (i >= vms_vector_size(buffers))
        i = 0;

    vms_event *inevent;

    while (i < vms_vector_size(buffers)) {
        buffer = ((vms_arbiter_buffer *)vms_vector_at(buffers, i));
        if (!vms_arbiter_buffer_active(buffer))
            continue;
        assert(buffer);
        stream = vms_arbiter_buffer_stream(buffer);
        ++i;

        qsize = vms_arbiter_buffer_peek1(buffer, (void **)&inevent);
        if (qsize > 0) {
            assert(shmn->_ev);
            const uint64_t c = vms_arbiter_buffer_capacity(buffer);
            /* is the buffer full from 75 or more percent? */
            if (qsize > 0.8 * c) {
                /* drop half of the buffer */
                vms_eventid id = vms_event_id(inevent);
                if (!vms_arbiter_buffer_drop(buffer, c / 2)) {
                    assert(0 && "Failed dropping events");
                }
                vms_stream_prepare_hole_event(stream, shmn->_ev, id, c / 2);
                *streamret = stream;
                return shmn->_ev;
            }

            /* TODO: ideally, we do not copy the event here but pass it directly
             * to the monitor */
            memcpy(shmn->_ev, inevent, vms_arbiter_buffer_elem_size(buffer));
#ifndef NDEBUG
            size_t n =
#endif
                vms_arbiter_buffer_drop(buffer, 1);
            assert(n == 1);
            *streamret = stream;
            return shmn->_ev;
        }
    }

    // TODO: we should check if the stream is finished and remove it
    // in that case
    return NULL;
}

shamon *shamon_create(shamon_process_events_fn process_events,
                      void *process_events_data) {
    shamon *shmn = malloc(sizeof(shamon));
    assert(shmn);

    VEC_INIT(shmn->streams);
    vms_vector_aligned_init(&shmn->buffers, vms_arbiter_buffer_sizeof(),
                            CACHELINE_SIZE);
    VEC_INIT(shmn->buffer_threads);
    shmn->_ev_size = sizeof(vms_event_default_hole);
    shmn->_ev = malloc(shmn->_ev_size);
    shmn->process_events =
        process_events ? process_events : default_process_events;
    shmn->process_events_data = process_events ? process_events_data : shmn;

    return shmn;
}

vms_vector *shamon_get_buffers(shamon *shmn) { return _buffers(shmn); }

vms_stream **shamon_get_streams(shamon *shmn, size_t *size) {
    *size = VEC_SIZE(shmn->streams);
    return shmn->streams;
}

/* detach streams, but do not destroy them
 * -- this is for use in signal handlers when
 * the program is about to be killed */
void shamon_detach(shamon *shmn) {
    for (size_t i = 0; i < VEC_SIZE(shmn->streams); ++i) {
        vms_stream_detach(shmn->streams[i]);
    }
}

void shamon_destroy(shamon *shmn) {
    assert(VEC_SIZE(shmn->buffer_threads) == vms_vector_size(_buffers(shmn)));
    for (size_t i = 0; i < VEC_SIZE(shmn->buffer_threads); ++i) {
        thrd_join(shmn->buffer_threads[i], NULL);
        /*
            vms_arbiter_buffer *buff
                = *((vms_arbiter_buffer **)vms_vector_at(_buffers(shmn), i));
            vms_arbiter_buffer_set_active(buff, false);
            vms_arbiter_buffer_destroy(buff);
        */
    }

    for (size_t i = 0; i < VEC_SIZE(shmn->streams); ++i) {
        if (!vms_stream_is_substream(shmn->streams[i])) {
            vms_stream_destroy(shmn->streams[i]);
        }
    }
    VEC_DESTROY(shmn->streams);
    VEC_DESTROY(shmn->buffer_threads);
    vms_vector_destroy(_buffers(shmn));
    free(shmn->_ev);
    free(shmn);
}

bool shamon_is_ready(shamon *shmn) {
    for (size_t i = 0; i < VEC_SIZE(shmn->streams); ++i) {
        vms_stream *s = shmn->streams[i];
        if (vms_stream_is_ready(s) || vms_stream_has_new_substreams(s)) {
            return true;
        } else {
            vms_arbiter_buffer *buff = vms_vector_at(_buffers(shmn), i);
            if (vms_arbiter_buffer_size(buff) > 0) {
                return true;
            }
        }
    }

    return false;
}

vms_event *shamon_get_next_ev(shamon *shmn, vms_stream **streamret) {
    for (size_t i = 0; i < VEC_SIZE(shmn->streams); ++i) {
        vms_stream *s = shmn->streams[i];
        vms_stream *new_stream =
            vms_stream_create_substream(s, NULL, NULL, NULL, NULL, NULL);
        if (new_stream) {
            fprintf(stderr, "Stream %lu has a new dynamic substream\n",
                    vms_stream_id(s));
            vms_stream_register_all_events(new_stream);
            vms_arbiter_buffer *buffer = vms_vector_at(_buffers(shmn), i);
            shamon_add_stream(shmn, new_stream,
                              vms_arbiter_buffer_capacity(buffer));
        }
    }

    return shmn->process_events(_buffers(shmn), shmn->process_events_data,
                                streamret);
}

void shamon_add_stream(shamon *shmn, vms_stream *stream,
                       size_t buffer_capacity) {
    for (unsigned i = 0; i < VEC_SIZE(shmn->streams); ++i) {
        if (strcmp(shmn->streams[i]->name, stream->name) == 0) {
            fprintf(stderr, "Stream '%s' added multiple times\n", stream->name);
            abort();
        }
    }

    VEC_PUSH(shmn->streams, &stream);
    assert(shmn->streams[VEC_SIZE(shmn->streams) - 1] == stream &&
           "BUG: vms_vector_push");

    size_t strm_event_size = stream->hole_handling.hole_event_size;
    if (stream->event_size > strm_event_size)
        strm_event_size = stream->event_size;
    if (strm_event_size > shmn->_ev_size) {
        shmn->_ev = realloc(shmn->_ev, strm_event_size);
        shmn->_ev_size = strm_event_size;
    }

    vms_arbiter_buffer *buffer = vms_vector_aligned_extend(_buffers(shmn));
    /* 0 as the output event size means same as the stream */
    vms_arbiter_buffer_init(buffer, stream,
                            /* output event size = */ 0, buffer_capacity);

    thrd_t thread_id;
    thrd_create(&thread_id, default_buffer_manager_thrd, buffer);
    VEC_PUSH(shmn->buffer_threads, &thread_id);

    printf("Added a stream id %lu: '%s'\n", VEC_SIZE(shmn->streams) - 1,
           stream->type);

    vms_arbiter_buffer_set_active(buffer, true);
}
