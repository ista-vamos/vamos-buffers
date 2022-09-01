#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "monitor.h"
#include "par_queue.h"
#include "utils.h"

#define SLEEP_TIME_NS_INIT 1
#define BUSY_WAIT_TIMES 1000
#define SLEEP_TIME_NS_THRES 1000000

typedef struct _shm_monitor_buffer {
    shm_par_queue buffer; // the buffer itself
    bool finished;        // the arbiter has finished?
} shm_monitor_buffer;

size_t shm_monitor_buffer_sizeof(void) {
    return sizeof(shm_monitor_buffer);
}
size_t shm_monitor_buffer_size(shm_monitor_buffer *buffer) {
    return shm_par_queue_size(&buffer->buffer);
}

size_t shm_monitor_buffer_free_space(shm_monitor_buffer *buffer) {
    return shm_par_queue_free_num(&buffer->buffer);
}

size_t shm_monitor_buffer_capacity(shm_monitor_buffer *buffer) {
    return shm_par_queue_capacity(&buffer->buffer);
}

void shm_monitor_buffer_init(shm_monitor_buffer *buffer, size_t event_size,
                             size_t capacity) {
    shm_par_queue_init(&buffer->buffer, capacity, event_size);
    buffer->finished = false;
}

void shm_monitor_set_finished(shm_monitor_buffer *buffer) {
    buffer->finished = true;
}

shm_monitor_buffer *shm_monitor_buffer_create(size_t event_size,
                                              size_t capacity) {
    shm_monitor_buffer *b = malloc(shm_monitor_buffer_sizeof());
    assert(b && "Malloc failed");

    shm_monitor_buffer_init(b, event_size, capacity);
    return b;
}

void shm_monitor_buffer_free(shm_monitor_buffer *buffer) {
    shm_monitor_buffer_destroy(buffer);
    free(buffer);
}

void shm_monitor_buffer_destroy(shm_monitor_buffer *buffer) {
    shm_par_queue_destroy(&buffer->buffer);
}

size_t shm_monitor_buffer_elem_size(shm_monitor_buffer *q) {
    return shm_par_queue_elem_size(&q->buffer);
}

shm_event *shm_monitor_buffer_top(shm_monitor_buffer *buffer) {
    return shm_par_queue_top(&buffer->buffer);
}

size_t shm_monitor_buffer_peek(shm_monitor_buffer *buffer, size_t n,
                               void **data1, size_t *size1, void **data2,
                               size_t *size2) {
    return shm_par_queue_peek(&buffer->buffer, n, data1, size1, data2, size2);
}

size_t shm_monitor_buffer_peek1(shm_monitor_buffer *buffer, void **data) {
    return shm_par_queue_peek1(&buffer->buffer, data);
}

<<<<<<< HEAD
void *shm_monitor_buffer_write_ptr_or_null(shm_monitor_buffer *q) {
    assert(!q->finished && "Asking a pointer from a finished buffer");
    return shm_par_queue_write_ptr(&q->buffer);
}

void *shm_monitor_buffer_write_ptr(shm_monitor_buffer *q) {
    assert(!q->finished && "Asking a pointer from a finished buffer");

=======
void *shm_monitor_buffer_write_ptr(shm_monitor_buffer *q) {
>>>>>>> 4140ff3 (core: add shm_monitor_buffer)
    void *ptr = shm_par_queue_write_ptr(&q->buffer);
    if (ptr)
        return ptr;

    /* wait for space in the buffer */
    size_t spinned = 0;
    uint64_t sleep_time = SLEEP_TIME_NS_INIT;
    do {
        if (++spinned > 1000) {
            if (sleep_time < SLEEP_TIME_NS_THRES)
                sleep_time *= 10;
            sleep_ns(sleep_time);
            /* TODO: should we use some signaling here after some time
             * of spinning/sleeping? */
        }

<<<<<<< HEAD
        assert(!q->finished && "Asking a pointer from a finished buffer");
=======
>>>>>>> 4140ff3 (core: add shm_monitor_buffer)
        ptr = shm_par_queue_write_ptr(&q->buffer);
    } while (!ptr);

    assert(ptr);
    return ptr;
}

void shm_monitor_buffer_write_finish(shm_monitor_buffer *q) {
<<<<<<< HEAD
    assert(!q->finished && "Asking a pointer from a finished buffer");
=======
>>>>>>> 4140ff3 (core: add shm_monitor_buffer)
    shm_par_queue_write_finish(&q->buffer);
}

/* get an event from the stream, block until there is some and return it
 * or return NULL if the stream ended */
<<<<<<< HEAD
void *fetch_arbiter_stream(shm_monitor_buffer *buffer) {
=======
void *fetch_arbiter_stream(shm_monitor_buffer *buffer, uint64_t timeout) {
>>>>>>> 4140ff3 (core: add shm_monitor_buffer)
    size_t sleep_time = SLEEP_TIME_NS_INIT;
    size_t spinned = 0;
    void *ev;

    while (1) {
        
        /* wait for the event */
        ev = shm_monitor_buffer_top(buffer);
        if (ev) {
            return ev;
        }

<<<<<<< HEAD
        if (buffer->finished) {
            return NULL;
        }

=======
>>>>>>> 4140ff3 (core: add shm_monitor_buffer)
        /* before sleeping, try just to busy wait some time */
        if (spinned < BUSY_WAIT_TIMES) {
            ++spinned;
            continue;
        }

        /* no event read, sleep a while */
        if (sleep_time < SLEEP_TIME_NS_THRES) {
            sleep_time *= 10;
        }

<<<<<<< HEAD
=======
        if (timeout > 0) {
            if (timeout < sleep_time)
                return NULL;
            timeout -= sleep_time;
        }

>>>>>>> 4140ff3 (core: add shm_monitor_buffer)
        sleep_ns(sleep_time);
    }

    assert(0 && "Unreachable");
}

<<<<<<< HEAD
size_t shm_monitor_buffer_consume(shm_monitor_buffer *buffer, size_t k) {
    return shm_par_queue_drop(&buffer->buffer, k);
}
=======
#if 0
/* drop an event and notify buffer the buffer that it may free up
 * the payload of this and older events */
size_t shm_monitor_buffer_drop(shm_monitor_buffer *buffer, size_t k) {
#ifdef DUMP_STATS
    buffer->volunt_dropped_num_asked += k;
#endif
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
#ifdef DUMP_STATS
    buffer->volunt_dropped_num += k;
#endif
    return k;
}
#endif
>>>>>>> 4140ff3 (core: add shm_monitor_buffer)

/* wait for an event on the 'stream'
void shm_monitor_buffer_notify_dropped(shm_monitor_buffer *buffer,
                                       uint64_t begin_id, uint64_t end_id) {
    shm_stream_notify_dropped(buffer->stream, begin_id, end_id);
}
*/
