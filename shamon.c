#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <threads.h>
#include <stdbool.h>

#include "shamon.h"
#include "stream.h"
#include "vector.h"
#include "queue.h"
#include "utils.h"

/*****
 * STREAMS
 *****/
typedef struct _shm_arbiter_buffer {
    shm_stream *stream; // the source for the buffer
    shm_queue buffer;   // the buffer itself
    size_t dropped_num; // the number of dropped events
    bool active;        // true while the events are being queued
} shm_arbiter_buffer;

static void shm_arbiter_buffer_init(shm_arbiter_buffer *buffer,
                                    shm_stream *stream) {
    const size_t capacity = 1024; // FIXME

    buffer->stream = stream;
    shm_queue_init(&buffer->buffer, capacity, stream->event_size);
    buffer->active = false;
    buffer->dropped_num = 0;
}

int buffer_manager(void *data) {
    shm_arbiter_buffer *buffer = (shm_arbiter_buffer*) data;
    shm_stream *stream = buffer->stream;
    shm_queue *queue = &buffer->buffer;
    size_t max_size = shm_queue_max_size(queue);

    // wait for buffer->active
    while (!buffer->active)
        ;

    printf("Running fill & autodrop for stream %s\n", stream->name);
    while (buffer->active) {
        if (stream->has_event(stream)) {
            shm_event *ev = stream->get_next_event(stream);
            assert(ev);
            if (max_size - shm_queue_size(queue) < 2) {
                ++buffer->dropped_num;
                continue;
            }

            if (buffer->dropped_num > 0) {
                assert(0 && "Send dropped event");
               //dropped.num = buffer->dropped_num;
               //shm_queue_push(queue, &dropped);
                buffer->dropped_num = 0;
            }

            printf("Buffering event from stream %s\n", stream->name);
            shm_queue_push(queue, ev);
        }
        sleep_ns(10000);
    }

    // TODO: we should check if the stream is finished and remove it
    // in that case
    return 0;
}

typedef struct _shamon {
        shm_vector streams;
        shm_vector buffers;
        shm_vector buffer_threads;
        // memory for passing the next event
        shm_event *_ev;
        size_t _ev_size;
} shamon;

shamon *shamon_create(void) {
        shamon *shmn = malloc(sizeof(shamon));
        assert(shmn);

        shm_vector_init(&shmn->streams, sizeof(shm_stream *));
        shm_vector_init(&shmn->buffers, sizeof(shm_arbiter_buffer));
        shm_vector_init(&shmn->buffer_threads, sizeof(thrd_t));
        shmn->_ev = NULL;
        shmn->_ev_size = 0;

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
    // use static counter to do round robin -- so that some stream
    // does not starve
    static unsigned i = 0;
    shm_arbiter_buffer *buffer = NULL;
    // reset counter if we're at the end
    if (i >= shm_vector_size(&shmn->buffers))
        i = 0;

    while (i < shm_vector_size(&shmn->buffers)) {
        buffer = ((shm_arbiter_buffer*)shm_vector_at(&shmn->buffers, i));
        assert(buffer);
        ++i;

        if (shm_queue_size(&buffer->buffer) > 0) {
            assert(shmn->_ev);
            shm_queue_pop(&buffer->buffer, shmn->_ev);
            return shmn->_ev;
        }
    }

    // TODO: we should check if the stream is finished and remove it
    // in that case
    return NULL;
}

void shamon_add_stream(shamon *shmn, shm_stream *stream) {
    shm_vector_push(&shmn->streams, &stream);
    assert((*((shm_stream**)shm_vector_at(&shmn->streams, shm_vector_size(&shmn->streams) - 1)) == stream)
           && "BUG: shm_vector_push");
    if (stream->event_size > shmn->_ev_size) {
        shmn->_ev = realloc(shmn->_ev, stream->event_size);
        shmn->_ev_size = stream->event_size;
    }

    shm_arbiter_buffer *buffer = shm_vector_extend(&shmn->buffers);
    shm_arbiter_buffer_init(buffer, stream);

    thrd_t thread_id;
    thrd_create(&thread_id, buffer_manager, buffer);
    shm_vector_push(&shmn->buffer_threads, &thread_id);
    buffer->active = true;

    printf("Added a stream id %lu: '%s'\n",
           shm_vector_size(&shmn->streams) - 1,
           stream->name);
}
