#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <threads.h>
#include <stdbool.h>
#include <stdatomic.h>

#include "shamon.h"
#include "stream.h"
#include "vector.h"
#include "vector-macro.h"
#include "parallel_queue.h"
#include "utils.h"
#include "arbiter.h"

typedef struct _shamon {
        VEC(streams, shm_stream *);
        shm_vector buffers;
        VEC(buffer_threads, thrd_t);
        /* callbacks and their data */
        shamon_process_events_fn process_events;
        void *process_events_data;
        /* the memory for passing the next event in the default
           process_events handler */
        shm_event *_ev;
        size_t _ev_size;
} shamon;


#define SLEEP_NS_INIT (50)
#define SLEEP_THRESHOLD_NS (10000000)

static int default_buffer_manager_thrd(void *data) {
    shm_arbiter_buffer *buffer = (shm_arbiter_buffer*) data;
    shm_stream *stream = shm_arbiter_buffer_stream(buffer);
    register shm_stream_alter_fn alter = stream->alter;
    register shm_stream_filter_fn filter = stream->filter;

    // wait for buffer->active
    while (!shm_arbiter_buffer_active(buffer))
        ;

    printf("Running fetch & autodrop for stream %s\n", stream->name);

    void *ev, *out;
    while (shm_stream_is_ready(stream)) {
        ev = stream_fetch(stream, buffer);
        if (!ev) {
            continue;
        }
        if (filter && !filter(stream, ev)) {
            shm_stream_consume(stream, 1);
            continue;
        }

        out = shm_arbiter_buffer_write_ptr(buffer);
        assert(out && "No space in the buffer");
        alter(stream, ev, out);
        shm_arbiter_buffer_write_finish(buffer);
        shm_stream_consume(stream, 1);
    }

    // TODO: we should check if the stream is finished and remove it
    // in that case
    printf("BMM for stream %lu (%s) exits\n", stream->id, stream->name);
    thrd_exit(EXIT_SUCCESS);
}


static
shm_event *default_process_events(shm_vector *buffers, void *data) {
    assert(buffers);
    assert(data);
    shamon *shmn = (shamon *)data;
    shm_stream *stream;

    // use static counter to do round robin -- so that some stream
    // does not starve
    static unsigned i = 0;
    shm_arbiter_buffer *buffer = NULL;
    size_t qsize;
    shm_event_dropped dropped;

    // reset counter if we're at the end
    if (i >= shm_vector_size(buffers))
        i = 0;

    while (i < shm_vector_size(buffers)) {
        buffer = ((shm_arbiter_buffer*)shm_vector_at(buffers, i));
        if (!shm_arbiter_buffer_active(buffer))
            continue;
        assert(buffer);
        stream = shm_arbiter_buffer_stream(buffer);
        ++i;

        qsize = shm_arbiter_buffer_size(buffer);
        if (qsize > 0) {
            assert(shmn->_ev);
            uint64_t c = shm_arbiter_buffer_capacity(buffer);
            /* is the buffer full from 75 or more percent? */
            if (qsize > 0.75*c) {
                /* drop half of the buffer */
                shm_eventid id = shm_event_id(shm_arbiter_buffer_top(buffer));
                if (!shm_arbiter_buffer_drop(buffer, c/4)) {
                    assert(0 && "Failed dropping events");
                }
                shm_stream_get_dropped_event(stream, &dropped, id, c/4);
                assert(shmn->_ev_size >= sizeof(dropped));
                memcpy(shmn->_ev, &dropped, sizeof(dropped));
                return shmn->_ev;
            }

            /* TODO: ideally, we do not copy the event here */
            shm_arbiter_buffer_pop(buffer, shmn->_ev);
            return shmn->_ev;
        }
    }

    // TODO: we should check if the stream is finished and remove it
    // in that case
    return NULL;
}

shamon *shamon_create(shamon_process_events_fn process_events,
                      void *process_events_data) {
        initialize_events();

        shamon *shmn = malloc(sizeof(shamon));
        assert(shmn);

        VEC_INIT(shmn->streams);
        shm_vector_init(&shmn->buffers, shm_arbiter_buffer_sizeof());
        VEC_INIT(shmn->buffer_threads);
        shmn->_ev_size = sizeof(shm_event_dropped);
        shmn->_ev = malloc(shmn->_ev_size);
        shmn->process_events = process_events ? process_events : default_process_events;
        shmn->process_events_data = process_events ? process_events_data : shmn;

        return shmn;
}

shm_vector *shamon_get_buffers(shamon *shmn) {
    return &shmn->buffers;
}

void shamon_destroy(shamon *shmn) {
    VEC_DESTROY(shmn->streams);
    for (size_t i = 0; i < VEC_SIZE(shmn->buffer_threads); ++i) {
        shm_arbiter_buffer *buff
            = *((shm_arbiter_buffer **)shm_vector_at(&shmn->buffers, i));
        shm_arbiter_buffer_set_active(buff, false);
        thrd_join(shmn->buffer_threads[i], NULL);
        //shm_arbiter_buffer_destroy(buff);
    }
    VEC_DESTROY(shmn->buffer_threads);
    shm_vector_destroy(&shmn->buffers);
    free(shmn->_ev);
    free(shmn);

    deinitialize_events();
}

bool shamon_is_ready(shamon *shmn) {
    for (size_t i = 0; i < VEC_SIZE(shmn->streams); ++i) {
        shm_stream *s = shmn->streams[i];
        if (shm_stream_is_ready(s)) {
            return true;
        }/* else {
            shm_arbiter_buffer *buff
                = *((shm_arbiter_buffer **)shm_vector_at(&shmn->buffers, i));
            shm_arbiter_buffer_set_active(buff, false);
        }
        */
    }

    return false;
}

shm_event *shamon_get_next_ev(shamon *shmn) {
    return shmn->process_events(&shmn->buffers, shmn->process_events_data);
}

void shamon_add_stream(shamon *shmn, shm_stream *stream) {
    VEC_PUSH(shmn->streams, &stream);
    assert(shmn->streams[VEC_SIZE(shmn->streams) - 1] == stream
            && "BUG: shm_vector_push");
    if (stream->event_size > shmn->_ev_size) {
        shmn->_ev = realloc(shmn->_ev, stream->event_size);
        shmn->_ev_size = stream->event_size;
    }

    shm_arbiter_buffer *buffer = shm_vector_extend(&shmn->buffers);
    shm_arbiter_buffer_init(buffer, stream);

    thrd_t thread_id;
    thrd_create(&thread_id, default_buffer_manager_thrd, buffer);
    VEC_PUSH(shmn->buffer_threads, &thread_id);

    printf("Added a stream id %lu: '%s'\n",
           VEC_SIZE(shmn->streams) - 1,
           stream->name);

    shm_arbiter_buffer_set_active(buffer, true);
}
