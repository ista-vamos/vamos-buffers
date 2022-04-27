#include <assert.h>

#include "stream.h"
#include "arbiter.h"
#include "parallel_queue.h"

/* TODO move to its own file */
typedef struct _shm_arbiter_buffer {
    shm_stream *stream;     // the source for the buffer
    shm_par_queue buffer;   // the buffer itself
    size_t dropped_num;     // the number of dropped events
    bool active;            // true while the events are being queued
} shm_arbiter_buffer;

size_t shm_arbiter_buffer_sizeof(void) {
    return sizeof(shm_arbiter_buffer);
}

void shm_arbiter_buffer_push_k(shm_arbiter_buffer *q, const void *elems, size_t size);

size_t shm_arbiter_buffer_size(shm_arbiter_buffer *buffer) {
    return shm_par_queue_size(&buffer->buffer);
}

size_t shm_arbiter_buffer_capacity(shm_arbiter_buffer *buffer) {
    return shm_par_queue_capacity(&buffer->buffer);
}

bool shm_arbiter_buffer_drop(shm_arbiter_buffer *buffer, size_t k) {
    return shm_par_queue_drop(&buffer->buffer, k);
}

bool shm_arbiter_buffer_active(shm_arbiter_buffer *buffer)
{
    return buffer->active;
}

void shm_arbiter_buffer_set_active(shm_arbiter_buffer *buffer, bool val) {
    buffer->active = val;
}

shm_stream *shm_arbiter_buffer_stream(shm_arbiter_buffer *buffer)
{
    return buffer->stream;
}

void shm_arbiter_buffer_init(shm_arbiter_buffer *buffer,
                             shm_stream *stream) {
    const size_t capacity = 16*4096; // FIXME

    buffer->stream = stream;
    assert(capacity >= 3 && "We need at least 3 elements in the buffer");
    shm_par_queue_init(&buffer->buffer, capacity,
                       /* the buffer must be able to contain the event 'dropped' */
                       stream->event_size < sizeof(shm_event_dropped) ?
                           sizeof(shm_event_dropped) : stream->event_size);
    buffer->active = false;
    buffer->dropped_num = 0;
}

size_t shm_arbiter_buffer_elem_size(shm_arbiter_buffer *q) {
    return shm_par_queue_elem_size(&q->buffer);
}

void shm_arbiter_buffer_push(shm_arbiter_buffer *buffer, const void *elem, size_t size) {
    assert(shm_arbiter_buffer_active(buffer));
    shm_par_queue *queue = &buffer->buffer;

    if (buffer->dropped_num > 0) {
        if (shm_par_queue_free_num(queue) < 2) {
            ++buffer->dropped_num;
        } else {
            shm_event_dropped dropped;
            bool ret;

            shm_stream_get_dropped_event(buffer->stream, &dropped, buffer->dropped_num);
            assert(sizeof(dropped) <= shm_par_queue_elem_size(queue));
            ret = shm_par_queue_push(queue, &dropped, sizeof(dropped));
            assert(ret && "BUG: queue has not enough free space");
            ret = shm_par_queue_push(&buffer->buffer, elem, size);
            assert(ret && "BUG: queue has not enough free space");
            buffer->dropped_num = 0;
        }
    } else {
        if (!shm_par_queue_push(&buffer->buffer, elem, size)) {
            ++buffer->dropped_num;
        }
    }
}

/*
 * Push k events that reside in a continuous memory
 */
void shm_arbiter_buffer_push_k(shm_arbiter_buffer *buffer,
                               const void *elems,
                               size_t k) {
    //printf("Buffering %lu events\n", k);
    assert(shm_arbiter_buffer_active(buffer));
    shm_par_queue *queue = &buffer->buffer;

    if (buffer->dropped_num > 0) {
        if (shm_par_queue_free_num(queue) < 2) {
            //printf("[buffer] dropping %lu events (free space: %lu)\n", k, shm_par_queue_free_num(queue));
            buffer->dropped_num += k;
        } else {
            //printf("[buffer] generating dropped event 'dropped(%lu)'\n", buffer->dropped_num);
            shm_event_dropped dropped;

            shm_stream_get_dropped_event(buffer->stream, &dropped, buffer->dropped_num);
            assert(sizeof(dropped) <= shm_par_queue_elem_size(queue));
            assert(shm_par_queue_free_num(queue) > 1);
            shm_par_queue_push(queue, &dropped, sizeof(dropped));
            assert(shm_par_queue_free_num(queue) >= 1);
            buffer->dropped_num = shm_par_queue_push_k(&buffer->buffer, elems, k);
        }
    } else {
        buffer->dropped_num = shm_par_queue_push_k(&buffer->buffer, elems, k);
    }
}


bool shm_arbiter_buffer_pop(shm_arbiter_buffer *buffer, void *elem) {
    return shm_par_queue_pop(&buffer->buffer, elem);
}
