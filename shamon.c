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
#include "parallel_queue.h"
#include "utils.h"
#include "arbiter.h"

typedef struct _shamon {
        shm_vector streams;
        shm_vector buffers;
        shm_vector buffer_threads;
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

int buffer_manager_thrd(void *data) {
    shm_arbiter_buffer *buffer = (shm_arbiter_buffer*) data;
    shm_stream *stream = shm_arbiter_buffer_stream(buffer);

    // wait for buffer->active
    while (!shm_arbiter_buffer_active(buffer))
        ;

    printf("Running fill & autodrop for stream %s\n", stream->name);
    uint64_t sleep_time = SLEEP_NS_INIT;
    uint64_t n;
    while (shm_stream_is_ready(stream)) {
        n = stream->buffer_events(stream, buffer);
        if (n == 0) {
            if (sleep_time < SLEEP_THRESHOLD_NS)
                sleep_time *= 10;
            sleep_ns(sleep_time);
        } else {
            sleep_time = SLEEP_NS_INIT;
        }
    }

    // TODO: we should check if the stream is finished and remove it
    // in that case
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
            /* is the buffer full from 80 or more percent? */
            if (qsize > 0.5*c) {
                /* drop half of the buffer */
                if (!shm_arbiter_buffer_drop(buffer, c/4)) {
                    assert(0 && "Failed dropping events");
                }
                shm_stream_get_dropped_event(stream, &dropped, c/4);
                assert(shmn->_ev_size >= sizeof(dropped));
                memcpy(shmn->_ev, &dropped, sizeof(dropped));
                return shmn->_ev;
            }

            shm_arbiter_buffer_pop(buffer, shmn->_ev);
            /* TODO: refactor this */
            if (stream->publish_event)
                return stream->publish_event(stream, shmn->_ev);
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

        shm_vector_init(&shmn->streams, sizeof(shm_stream *));
        shm_vector_init(&shmn->buffers, shm_arbiter_buffer_sizeof());
        shm_vector_init(&shmn->buffer_threads, sizeof(thrd_t));
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
    shm_vector_destroy(&shmn->streams);
    for (size_t i = 0; i < shm_vector_size(&shmn->buffer_threads); ++i) {
        shm_arbiter_buffer *buff
            = *((shm_arbiter_buffer **)shm_vector_at(&shmn->buffers, i));
        shm_arbiter_buffer_set_active(buff, false);
        thrd_join(*(thrd_t*)shm_vector_at(&shmn->buffer_threads, i), NULL);
        //shm_arbiter_buffer_destroy(buff);
    }
    shm_vector_destroy(&shmn->buffer_threads);
    shm_vector_destroy(&shmn->buffers);
    free(shmn->_ev);
    free(shmn);

    deinitialize_events();
}

bool shamon_is_ready(shamon *shmn) {
    for (size_t i = 0; i < shm_vector_size(&shmn->streams); ++i) {
        shm_stream *s = *((shm_stream **)shm_vector_at(&shmn->streams, i));
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
    shm_vector_push(&shmn->streams, &stream);
    assert((*((shm_stream**)shm_vector_at(&shmn->streams,
                                          shm_vector_size(&shmn->streams) - 1))
            == stream) && "BUG: shm_vector_push");
    if (stream->event_size > shmn->_ev_size) {
        shmn->_ev = realloc(shmn->_ev, stream->event_size);
        shmn->_ev_size = stream->event_size;
    }

    shm_arbiter_buffer *buffer = shm_vector_extend(&shmn->buffers);
    shm_arbiter_buffer_init(buffer, stream);

    thrd_t thread_id;
    thrd_create(&thread_id, buffer_manager_thrd, buffer);
    shm_vector_push(&shmn->buffer_threads, &thread_id);

    printf("Added a stream id %lu: '%s'\n",
           shm_vector_size(&shmn->streams) - 1,
           stream->name);

    shm_arbiter_buffer_set_active(buffer, true);
}
