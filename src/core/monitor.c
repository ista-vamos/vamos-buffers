#include "vamos-buffers/core/monitor.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "vamos-buffers/core/par_queue.h"
#include "vamos-buffers/core/utils.h"

typedef struct _vms_monitor_buffer {
    vms_par_queue buffer;  // the buffer itself
    bool finished;         // the arbiter has finished?
} vms_monitor_buffer;

size_t vms_monitor_buffer_sizeof(void) { return sizeof(vms_monitor_buffer); }
size_t vms_monitor_buffer_size(vms_monitor_buffer *buffer) {
    return vms_par_queue_size(&buffer->buffer);
}

size_t vms_monitor_buffer_free_space(vms_monitor_buffer *buffer) {
    return vms_par_queue_free_num(&buffer->buffer);
}

size_t vms_monitor_buffer_capacity(vms_monitor_buffer *buffer) {
    return vms_par_queue_capacity(&buffer->buffer);
}

void vms_monitor_buffer_init(vms_monitor_buffer *buffer, size_t event_size,
                             size_t capacity) {
    vms_par_queue_init(&buffer->buffer, capacity, event_size);
    buffer->finished = false;
}

void vms_monitor_set_finished(vms_monitor_buffer *buffer) {
    buffer->finished = true;
}

vms_monitor_buffer *vms_monitor_buffer_create(size_t event_size,
                                              size_t capacity) {
    vms_monitor_buffer *b = malloc(vms_monitor_buffer_sizeof());
    assert(b && "Malloc failed");

    vms_monitor_buffer_init(b, event_size, capacity);
    return b;
}

void vms_monitor_buffer_free(vms_monitor_buffer *buffer) {
    vms_monitor_buffer_destroy(buffer);
    free(buffer);
}

void vms_monitor_buffer_destroy(vms_monitor_buffer *buffer) {
    vms_par_queue_destroy(&buffer->buffer);
}

size_t vms_monitor_buffer_elem_size(vms_monitor_buffer *q) {
    return vms_par_queue_elem_size(&q->buffer);
}

vms_event *vms_monitor_buffer_top(vms_monitor_buffer *buffer) {
    return vms_par_queue_top(&buffer->buffer);
}

size_t vms_monitor_buffer_peek(vms_monitor_buffer *buffer, size_t n,
                               void **data1, size_t *size1, void **data2,
                               size_t *size2) {
    return vms_par_queue_peek(&buffer->buffer, n, data1, size1, data2, size2);
}

size_t vms_monitor_buffer_peek1(vms_monitor_buffer *buffer, void **data) {
    return vms_par_queue_peek1(&buffer->buffer, data);
}

void *vms_monitor_buffer_write_ptr_or_null(vms_monitor_buffer *q) {
    assert(!q->finished && "Asking a pointer from a finished buffer");
    return vms_par_queue_write_ptr(&q->buffer);
}

void *vms_monitor_buffer_write_ptr(vms_monitor_buffer *q) {
    assert(!q->finished && "Asking a pointer from a finished buffer");

    void *ptr = vms_par_queue_write_ptr(&q->buffer);
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
        ptr = vms_par_queue_write_ptr(&q->buffer);
    } while (!ptr);

    assert(ptr);
    return ptr;
}

void vms_monitor_buffer_write_finish(vms_monitor_buffer *q) {
    assert(!q->finished && "Asking a pointer from a finished buffer");
    vms_par_queue_write_finish(&q->buffer);
}

/* get an event from the stream, block until there is some and return it
 * or return NULL if the stream ended */
void *fetch_arbiter_stream(vms_monitor_buffer *buffer) {
    size_t sleep_time = SLEEP_TIME_INIT_NS;
    size_t spinned = 0;
    void *ev;

    while (1) {
        /* wait for the event */
        if ((ev = vms_monitor_buffer_top(buffer))) {
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

void vms_monitor_buffer_consume(vms_monitor_buffer *buffer, size_t k) {
    vms_par_queue_drop(&buffer->buffer, k);
}

/* wait for an event on the 'stream'
void vms_monitor_buffer_notify_dropped(vms_monitor_buffer *buffer,
                                       uint64_t begin_id, uint64_t end_id) {
    vms_stream_notify_dropped(buffer->stream, begin_id, end_id);
}
*/
