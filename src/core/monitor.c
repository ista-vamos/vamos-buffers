#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "vamos-buffers/core/monitor.h"
#include "vamos-buffers/core/par_queue.h"
#include "vamos-buffers/core/utils.h"

typedef struct _shm_monitor_buffer {
    shm_par_queue buffer;  // the buffer itself
    bool finished;         // the arbiter has finished?
} shm_monitor_buffer;

size_t shm_monitor_buffer_sizeof(void) { return sizeof(shm_monitor_buffer); }
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

void *shm_monitor_buffer_write_ptr_or_null(shm_monitor_buffer *q) {
    assert(!q->finished && "Asking a pointer from a finished buffer");
    return shm_par_queue_write_ptr(&q->buffer);
}

void *shm_monitor_buffer_write_ptr(shm_monitor_buffer *q) {
    assert(!q->finished && "Asking a pointer from a finished buffer");

    void *ptr = shm_par_queue_write_ptr(&q->buffer);
    if (ptr)
        return ptr;

        /* wait for space in the buffer */
#if 0
    size_t spinned = 0;
    uint64_t sleep_time = SLEEP_TIME_INIT_NS;
#endif
    do {
#if 0
        if (++spinned > BUSY_WAIT_FOR_EVENTS) {
            sleep_ns(sleep_time);
            if (sleep_time < SLEEP_TIME_THRES_NS)
                sleep_time *= 2;
            /* TODO: should we use some signaling here after some time
             * of spinning/sleeping? */
        }
#endif

        assert(!q->finished && "Asking a pointer from a finished buffer");
        ptr = shm_par_queue_write_ptr(&q->buffer);
    } while (!ptr);

    assert(ptr);
    return ptr;
}

void shm_monitor_buffer_write_finish(shm_monitor_buffer *q) {
    assert(!q->finished && "Asking a pointer from a finished buffer");
    shm_par_queue_write_finish(&q->buffer);
}

/* get an event from the stream, block until there is some and return it
 * or return NULL if the stream ended */
void *fetch_arbiter_stream(shm_monitor_buffer *buffer) {
    size_t sleep_time = SLEEP_TIME_INIT_NS;
    size_t spinned = 0;
    void *ev;

    while (1) {
        /* wait for the event */
        if ((ev = shm_monitor_buffer_top(buffer))) {
            return ev;
        }

        if (buffer->finished) {
            return NULL;
        }

        /* before sleeping, try just to busy wait some time */
        if (++spinned > BUSY_WAIT_FOR_EVENTS) {
            sleep_ns(sleep_time);
            /* no event read, sleep a while */
            if (sleep_time < SLEEP_TIME_THRES_NS) {
                sleep_time *= 2;
            }
        }
    }

    assert(0 && "Unreachable");
    abort();
}

void shm_monitor_buffer_consume(shm_monitor_buffer *buffer, size_t k) {
    shm_par_queue_drop(&buffer->buffer, k);
}

/* wait for an event on the 'stream'
void shm_monitor_buffer_notify_dropped(shm_monitor_buffer *buffer,
                                       uint64_t begin_id, uint64_t end_id) {
    shm_stream_notify_dropped(buffer->stream, begin_id, end_id);
}
*/
