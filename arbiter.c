#include <assert.h>

#include "utils.h"
#include "stream.h"
#include "arbiter.h"
#include "parallel_queue.h"

/* TODO move to its own file */
typedef struct _shm_arbiter_buffer {
    shm_stream *stream;     // the source for the buffer
    shm_par_queue buffer;   // the buffer itself
    size_t dropped_num;     // the number of dropped events
    shm_eventid dropped_id; // the id of the next 'dropped' event
    bool active;            // true while the events are being queued
} shm_arbiter_buffer;

size_t shm_arbiter_buffer_sizeof(void) {
    return sizeof(shm_arbiter_buffer);
}

void *shm_arbiter_buffer_write_ptr(shm_arbiter_buffer *q) {
    return shm_par_queue_write_ptr(&q->buffer);
}

void shm_arbiter_buffer_write_finish(shm_arbiter_buffer *q) {
    shm_par_queue_write_finish(&q->buffer);
}

void shm_arbiter_buffer_finish_push(shm_arbiter_buffer *q);

void shm_arbiter_buffer_push_k(shm_arbiter_buffer *q, const void *elems, size_t size);

size_t shm_arbiter_buffer_size(shm_arbiter_buffer *buffer) {
    return shm_par_queue_size(&buffer->buffer);
}

size_t shm_arbiter_buffer_free_space(shm_arbiter_buffer *buffer) {
    return shm_par_queue_free_num(&buffer->buffer);
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
    const size_t capacity = 4*4096; // FIXME

    buffer->stream = stream;
    assert(capacity >= 3 && "We need at least 3 elements in the buffer");
    shm_par_queue_init(&buffer->buffer, capacity,
                       /* the buffer must be able to contain the event 'dropped' */
                       stream->event_size < sizeof(shm_event_dropped) ?
                           sizeof(shm_event_dropped) : stream->event_size);
    buffer->active = false;
    buffer->dropped_num = 0;
}

void shm_arbiter_buffer_destroy(shm_arbiter_buffer *buffer) {
    shm_par_queue_destroy(&buffer->buffer);
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

            shm_stream_get_dropped_event(buffer->stream, &dropped,
                                         buffer->dropped_id, buffer->dropped_num);
            assert(sizeof(dropped) <= shm_par_queue_elem_size(queue));
            ret = shm_par_queue_push(queue, &dropped, sizeof(dropped));
            assert(ret && "BUG: queue has not enough free space");
            ret = shm_par_queue_push(&buffer->buffer, elem, size);
            assert(ret && "BUG: queue has not enough free space");
            buffer->dropped_num = 0;
        }
    } else {
        if (!shm_par_queue_push(&buffer->buffer, elem, size)) {
            buffer->dropped_id = shm_event_id((shm_event*)elem);
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

            shm_stream_get_dropped_event(buffer->stream, &dropped,
                                         buffer->dropped_id, buffer->dropped_num);
            assert(sizeof(dropped) <= shm_par_queue_elem_size(queue));
            assert(shm_par_queue_free_num(queue) > 1);
            shm_par_queue_push(queue, &dropped, sizeof(dropped));
            assert(shm_par_queue_free_num(queue) >= 1);
            buffer->dropped_num = shm_par_queue_push_k(&buffer->buffer, elems, k);
        }
    } else {
        buffer->dropped_id = shm_event_id((shm_event*)elems);
        buffer->dropped_num = shm_par_queue_push_k(&buffer->buffer, elems, k);
    }
}


bool shm_arbiter_buffer_pop(shm_arbiter_buffer *buffer, void *elem) {
    return shm_par_queue_pop(&buffer->buffer, elem);
}

shm_event *shm_arbiter_buffer_top(shm_arbiter_buffer *buffer) {
    return shm_par_queue_top(&buffer->buffer);
}

size_t shm_arbiter_buffer_peek(shm_arbiter_buffer *buffer, size_t n,
                               void **data1, size_t *size1,
                               void **data2, size_t *size2) {
    return shm_par_queue_peek(&buffer->buffer, n, data1, size1, data2, size2);
}

#define SLEEP_TIME_NS_INIT 10
#define SLEEP_TIME_NS_THRES 10000

/* wait for an event on the 'stream' and return a pointer for it
 * in 'in' param. Also set 'out' to point where this event should be
 * written if it is not dropped. 'pushed' means that the lastly
 * fetched event was pushed to the arbiter buffer*/
void *stream_fetch(shm_stream *stream,
                   shm_arbiter_buffer *buffer) {
    static shm_event_dropped dropped_ev;

    /* wait for the event */
    /* FIXME: if there is no filtering, we can push multiple events forward */
    size_t num = 1;
    size_t sleep_time = SLEEP_TIME_NS_INIT;
    void *ev;
    do {
        ev = shm_stream_read_events(stream, &num);
        if (ev) {
            sleep_time = SLEEP_TIME_NS_INIT;
            break;
        }
        if (sleep_time < SLEEP_TIME_NS_THRES)
            sleep_time *= 10;
        sleep_ns(sleep_time);
    } while (shm_stream_is_ready(stream));
    if (!ev)
        return NULL;

    /*
    printf("FETCH: read event { kind = %lu, id = %lu}\n",
           ((shm_event*)ev)->kind,
           ((shm_event*)ev)->id);*/
    if (buffer->dropped_num > 0) {
        if (shm_arbiter_buffer_free_space(buffer) > 1) {
            shm_stream_get_dropped_event(stream, &dropped_ev,
                                         buffer->dropped_id, buffer->dropped_num);
            shm_par_queue_push(&buffer->buffer, &dropped_ev, sizeof(dropped_ev));
            buffer->dropped_num = 0;
            assert(shm_arbiter_buffer_free_space(buffer) > 0);
            return ev;
        }

        ++buffer->dropped_num;
        return NULL;
    }

    if (shm_arbiter_buffer_free_space(buffer) == 0) {
        buffer->dropped_id = shm_event_id(ev);
        ++buffer->dropped_num;
        return NULL;
    }

    return ev;
}
