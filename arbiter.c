#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "utils.h"
#include "stream.h"
#include "arbiter.h"
#include "parallel_queue.h"

#define DROP_SPACE_THRESHOLD 1

typedef struct _shm_arbiter_buffer {
    shm_stream *stream;     // the source for the buffer
    shm_par_queue buffer;   // the buffer itself
    size_t dropped_num;     // the number of dropped events
    size_t total_dropped_num;     // the number of dropped events
    shm_eventid drop_begin_id; // the id of the next 'dropped' event
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

/* drop an event and notify buffer the buffer that it may free up
 * the payload of this and older events */
size_t shm_arbiter_buffer_drop(shm_arbiter_buffer *buffer, size_t k) {
    --k; /* peek_*_at takes index from 0 */
    shm_event *ev = shm_par_queue_peek_atmost_at(&buffer->buffer, &k);
    if (!ev)
        return 0; /* empty queue */
    shm_eventid last_id = shm_event_id(ev);
#ifndef NDEBUG
    size_t n =
#endif
    ++k; /* k is index, we must increase it back by one */
    shm_par_queue_drop(&buffer->buffer, k);
    assert(n == k && "Something changed the queue in between");
    shm_stream_notify_last_processed_id(buffer->stream, last_id);
    return k;
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

size_t shm_arbiter_buffer_dropped_num(shm_arbiter_buffer *buffer)
{
    return buffer->total_dropped_num;
}

void shm_arbiter_buffer_init(shm_arbiter_buffer *buffer,
                             shm_stream *stream,
                             size_t out_event_size,
                             size_t capacity) {
    assert(capacity >= 3 && "We need at least 3 elements in the buffer");
    size_t event_size = out_event_size > stream->event_size ? out_event_size : stream->event_size;
    /* the buffer must be able to contain the event 'dropped' */
    if (event_size < sizeof(shm_event_dropped))
        event_size = sizeof(shm_event_dropped);

    shm_par_queue_init(&buffer->buffer, capacity,
                       event_size);

    buffer->stream = stream;
    buffer->active = false;
    buffer->dropped_num = 0;
    buffer->total_dropped_num = 0;
}

shm_arbiter_buffer *shm_arbiter_buffer_create(shm_stream *stream,
                                              size_t out_event_size,
                                              size_t capacity) {
    shm_arbiter_buffer *b = malloc(shm_arbiter_buffer_sizeof());
    assert(b && "Malloc failed");
    shm_arbiter_buffer_init(b, stream, out_event_size, capacity);
    return b;
}
void shm_arbiter_buffer_free(shm_arbiter_buffer *buffer) {
    shm_arbiter_buffer_destroy(buffer);
    free(buffer);
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
            shm_stream_get_dropped_event(buffer->stream, &dropped,
                                         buffer->drop_begin_id,
                                         buffer->dropped_num);
            assert(sizeof(dropped) <= shm_par_queue_elem_size(queue));
#ifndef NDEBUG
            bool ret =
#endif
            shm_par_queue_push(queue, &dropped, sizeof(dropped));
            assert(ret && "BUG: queue has not enough free space");
#ifndef NDEBUG
            ret =
#endif
            shm_par_queue_push(&buffer->buffer, elem, size);
            assert(ret && "BUG: queue has not enough free space");
            buffer->total_dropped_num += buffer->dropped_num;
            buffer->dropped_num = 0;
            /* the end id may not be precise, but we need just the upper bound */
            shm_arbiter_buffer_notify_dropped(buffer, buffer->drop_begin_id,
                                              shm_event_id((shm_event*)elem) - 1);
        }
    } else {
        if (!shm_par_queue_push(&buffer->buffer, elem, size)) {
            buffer->drop_begin_id = shm_event_id((shm_event*)elem);
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
                                         buffer->drop_begin_id, buffer->dropped_num);
            assert(sizeof(dropped) <= shm_par_queue_elem_size(queue));
            assert(shm_par_queue_free_num(queue) > 1);
            shm_par_queue_push(queue, &dropped, sizeof(dropped));
            buffer->total_dropped_num += buffer->dropped_num;

            /* the end id may not be precise, but we need just the upper bound */
            shm_arbiter_buffer_notify_dropped(buffer, buffer->drop_begin_id,
                                              shm_event_id((shm_event*)elems) - 1);

            assert(shm_par_queue_free_num(queue) >= 1);
            buffer->dropped_num = shm_par_queue_push_k(&buffer->buffer, elems, k); 
        }
    } else {
        buffer->dropped_num = shm_par_queue_push_k(&buffer->buffer, elems, k);
        if (buffer->dropped_num > 0) {
            shm_event *first_dropped_ev
                    = shm_par_queue_peek_at(&buffer->buffer, k - buffer->dropped_num);
            assert(first_dropped_ev);
            buffer->drop_begin_id = shm_event_id((shm_event*)first_dropped_ev);
        }
    }
}


/* NOTE: does not notify about processing the event, must
 * be done manually once all the work with data is done */
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

size_t shm_arbiter_buffer_peek1(shm_arbiter_buffer *buffer, void **data) {
    return shm_par_queue_peek1(&buffer->buffer, data);
}


#define SLEEP_TIME_NS_INIT 10
#define SLEEP_TIME_NS_THRES 10000

static void *get_event(shm_stream *stream) {
    /* TODO: if there is no filtering and modifications, we can push multiple events forward.
             if there are filtering and modifications, we could have an additional thread
             to handle the load of data if we copy them in chunks */
    size_t num = 1;
    size_t sleep_time = SLEEP_TIME_NS_INIT;
    void *ev;
    while (1) {
        /* wait for the event */
        ev = shm_stream_read_events(stream, &num);
        if (ev) {
            sleep_time = SLEEP_TIME_NS_INIT;
            return ev;
        }

        /* no event read, sleep a while */
        if (sleep_time < SLEEP_TIME_NS_THRES) {
            sleep_time *= 10;
        } else {
            /* checking for the readiness is not cheap,
             * so do it only after we haven't read any
             * event for some time */
            if (!shm_stream_is_ready(stream))
                return NULL;
        }
        sleep_ns(sleep_time);
    }

    assert(0 && "Unreachable");
}

/* wait for an event on the 'stream' and return a pointer for it
 * in 'in' param. Also set 'out' to point where this event should be
 * written if it is not dropped. 'pushed' means that the lastly
 * fetched event was pushed to the arbiter buffer*/
void *stream_fetch(shm_stream *stream,
                   shm_arbiter_buffer *buffer) {
    static shm_event_dropped dropped_ev;
    void *ev;
    while (1) {
        ev = get_event(stream);
        if (!ev)
            return NULL; /* stream ended */

        assert(shm_event_id(ev) == ++stream->last_event_id &&
               "IDs are inconsistent");
        /*
        printf("FETCH: read event { kind = %lu, id = %lu}\n",
               ((shm_event*)ev)->kind,
               ((shm_event*)ev)->id);*/
        /*
        fprintf(stderr, "%d: event = {%lu, %lu}\n", __LINE__,
                shm_event_kind(ev), shm_event_id(ev));
                */
        if (buffer->dropped_num > 0) {
            assert(DROP_SPACE_THRESHOLD < shm_arbiter_buffer_capacity(buffer));
            if (shm_arbiter_buffer_free_space(buffer) > DROP_SPACE_THRESHOLD) {
                shm_stream_get_dropped_event(stream, &dropped_ev,
                                             buffer->drop_begin_id, buffer->dropped_num);
                shm_par_queue_push(&buffer->buffer, &dropped_ev, sizeof(dropped_ev));
                buffer->total_dropped_num += buffer->dropped_num;
                buffer->dropped_num = 0;
                /* the end id may not be precise, but we need just the upper bound */
                shm_arbiter_buffer_notify_dropped(buffer,
                                                  buffer->drop_begin_id,
                                                  shm_event_id(ev) - 1);
                assert(shm_arbiter_buffer_free_space(buffer) > 0);
                return ev;
            }

            ++buffer->dropped_num;
            /* notify about dropped events continuously, because it may take
             * long time to generate the dropped event */
            /* FIXME: % is slow... and make it configurable */
            if (buffer->dropped_num % 10000 == 0) {
                shm_arbiter_buffer_notify_dropped(buffer,
                                                  buffer->drop_begin_id,
                                                  shm_event_id(ev) - 1);
            }
            /* consume the dropped event */
            shm_stream_consume(stream, 1);
            continue;
        }

        assert(buffer->dropped_num == 0);
        if (shm_arbiter_buffer_free_space(buffer) == 0) {
            buffer->drop_begin_id = shm_event_id(ev);
            assert(buffer->dropped_num == 0);
            ++buffer->dropped_num;
            shm_stream_consume(stream, 1);
            continue;
        }

        return ev;
    }
}

void shm_arbiter_buffer_notify_dropped(shm_arbiter_buffer *buffer,
                                       uint64_t begin_id,
                                       uint64_t end_id) {
    shm_stream_notify_dropped(buffer->stream, begin_id, end_id);
}
