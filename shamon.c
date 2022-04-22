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


typedef struct _shm_arbiter_buffer {
    shm_stream *stream; // the source for the buffer
    shm_par_queue buffer;   // the buffer itself
    size_t dropped_num; // the number of dropped events
    bool active;        // true while the events are being queued
} shm_arbiter_buffer;

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

static void shm_arbiter_buffer_init(shm_arbiter_buffer *buffer,
                                    shm_stream *stream) {
    const size_t capacity = 1024; // FIXME

    buffer->stream = stream;
    shm_par_queue_init(&buffer->buffer, capacity,
                       /* the stream must be able to contain the event 'dropped' */
                       stream->event_size < sizeof(shm_event_dropped) ?
                           sizeof(shm_event_dropped) : stream->event_size);
    buffer->active = false;
    buffer->dropped_num = 0;
}

int buffer_manager_thrd(void *data) {
    shm_arbiter_buffer *buffer = (shm_arbiter_buffer*) data;
    shm_stream *stream = buffer->stream;
    shm_par_queue *queue = &buffer->buffer;
    size_t max_size = shm_par_queue_capacity(queue);
    shm_event_dropped dropped;

    // wait for buffer->active
    while (!buffer->active)
        ;

    printf("Running fill & autodrop for stream %s\n", stream->name);
    while (buffer->active) {
        if (stream->has_event(stream)) {
            shm_event *ev = stream->get_next_event(stream);
            assert(ev);
            if (max_size - shm_par_queue_size(queue) < 2) {
                ++buffer->dropped_num;
                continue;
            }

            if (buffer->dropped_num > 0) {
                shm_stream_get_dropped_event(buffer->stream, &dropped, buffer->dropped_num);
                assert(sizeof(dropped) <= shm_par_queue_elem_size(queue));
                shm_par_queue_push(queue, &dropped, sizeof(dropped));
                buffer->dropped_num = 0;
            }

            /* printf("Buffering event from stream %s\n", stream->name); */
            assert(shm_event_copy_fn(ev) == NULL && "Not implemented yet");
            shm_par_queue_push(queue, ev, shm_event_size(ev));
        }
        sleep_ns(10000);
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
        assert(buffer);
        ++i;

        qsize = shm_par_queue_size(&buffer->buffer);
        if (qsize > 0) {
            assert(shmn->_ev);
            /* is the buffer full from 80 or more percent? */
            if (qsize > (int)(0.8*shm_par_queue_capacity(&buffer->buffer))) {
                /* drop half of the buffer */
                uint64_t n = shm_par_queue_capacity(&buffer->buffer) / 2;
                if (!shm_par_queue_drop(&buffer->buffer, n)) {
                    assert(0 && "Failed dropping events");
                }
                shm_stream_get_dropped_event(buffer->stream, &dropped, n);
                assert(shmn->_ev_size >= sizeof(dropped));
                memcpy(shmn->_ev, &dropped, sizeof(dropped));
                return shmn->_ev;
            }

            shm_par_queue_pop(&buffer->buffer, shmn->_ev);
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
        shm_vector_init(&shmn->buffers, sizeof(shm_arbiter_buffer));
        shm_vector_init(&shmn->buffer_threads, sizeof(thrd_t));
        shmn->_ev = NULL;
        shmn->_ev_size = sizeof(shm_event_dropped);
        shmn->process_events = process_events ? process_events : default_process_events;
        shmn->process_events_data = process_events ? process_events_data : shmn;

        return shmn;
}

void shamon_destroy(shamon *shmn) {
        shm_vector_destroy(&shmn->streams);
        for (size_t i = 0; i < shm_vector_size(&shmn->buffer_threads); ++i) {
            shm_arbiter_buffer *buff = *((shm_arbiter_buffer **)shm_vector_at(&shmn->buffers, i));
            buff->active = false;
            thrd_join(*(thrd_t*)shm_vector_at(&shmn->buffer_threads, i), NULL);
        }
        shm_vector_destroy(&shmn->buffer_threads);
        shm_vector_destroy(&shmn->buffers);
        free(shmn->_ev);
        free(shmn);
}

shm_event *shamon_get_next_ev(shamon *shmn) {
    return shmn->process_events(&shmn->buffers, shmn->process_events_data);
}

void shamon_add_stream(shamon *shmn, shm_stream *stream) {
    shm_vector_push(&shmn->streams, &stream);
    assert((*((shm_stream**)shm_vector_at(&shmn->streams, shm_vector_size(&shmn->streams) - 1)) == stream)
           && "BUG: shm_vector_push");
    if ((shmn->_ev == NULL) || (stream->event_size > shmn->_ev_size)) {
        shmn->_ev = realloc(shmn->_ev, stream->event_size);
        shmn->_ev_size = stream->event_size;
    }

    shm_arbiter_buffer *buffer = shm_vector_extend(&shmn->buffers);
    shm_arbiter_buffer_init(buffer, stream);

    thrd_t thread_id;
    thrd_create(&thread_id, buffer_manager_thrd, buffer);
    shm_vector_push(&shmn->buffer_threads, &thread_id);
    buffer->active = true;

    printf("Added a stream id %lu: '%s'\n",
           shm_vector_size(&shmn->streams) - 1,
           stream->name);
}
