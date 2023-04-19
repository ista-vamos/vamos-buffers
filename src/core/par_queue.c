#include "vamos-buffers/core/par_queue.h"

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define __predict_false(x) __builtin_expect((x) != 0, 0)
#define __predict_true(x) __builtin_expect((x) != 0, 1)

void vms_par_queue_init(vms_par_queue *q, size_t capacity, size_t elem_size) {
    assert(q);
    assert(capacity > 0);
    assert(elem_size > 0);

    vms_spsc_ringbuf_init(&q->ringbuf, capacity + 1);

    q->capacity = capacity;
    q->elem_size = elem_size;
    q->data = malloc((capacity + 1) * elem_size);
    if (!q->data) {
        assert(false && "Allocation failed");
        abort();
    }
}

void vms_par_queue_destroy(vms_par_queue *q) { free(q->data); }

/* Pointer to the next writable slot */
void *vms_par_queue_write_ptr(vms_par_queue *q) {
    size_t n;
    size_t off = vms_spsc_ringbuf_write_off_nowrap(&q->ringbuf, &n);
    if (__predict_true(n > 0)) {
        return q->data + (off * q->elem_size);
    }
    return NULL;
}

void vms_par_queue_write_finish(vms_par_queue *q) {
    vms_spsc_ringbuf_write_finish(&q->ringbuf, 1);
}

/* push an element into the queue.
 * 'size' is the actual size of the pushed element
 * and it must hold that 'size' <= 'elem_size' */
bool vms_par_queue_push(vms_par_queue *q, const void *elem, size_t size) {
    assert(q->elem_size >= size && "Size does not fit the slot");

    void *ptr = vms_par_queue_write_ptr(q);
    if (__predict_true(ptr)) {
        memcpy(ptr, elem, size);
        vms_par_queue_write_finish(q);
        return true;
    }

    return false;
}

bool vms_par_queue_pop(vms_par_queue *q, void *buff) {
    size_t n;
    const size_t off = vms_spsc_ringbuf_read_off_nowrap(&q->ringbuf, &n);
    if (__predict_true(n > 0)) {
        memcpy(buff, q->data + (off * q->elem_size), q->elem_size);
        vms_spsc_ringbuf_consume(&q->ringbuf, 1);
        return true;
    }
    return false;
}

void vms_par_queue_drop(vms_par_queue *q, size_t k) {
    vms_spsc_ringbuf_consume(&q->ringbuf, k);
}

size_t vms_par_queue_free_num(vms_par_queue *q) {
    return vms_spsc_ringbuf_free_num(&q->ringbuf);
}

size_t vms_par_queue_capacity(vms_par_queue *q) { return q->capacity; }

size_t vms_par_queue_size(vms_par_queue *q) {
    return vms_spsc_ringbuf_size(&q->ringbuf);
}

size_t vms_par_queue_elem_size(vms_par_queue *q) { return q->elem_size; }

vms_event *vms_par_queue_top(vms_par_queue *q) {
    size_t n;
    const size_t off = vms_spsc_ringbuf_read_off_nowrap(&q->ringbuf, &n);
    if (__predict_true(n > 0)) {
        return (vms_event *)(q->data + (off * q->elem_size));
    }
    return (vms_event *)0;
}

/* n == 0 means arbitrary n */
size_t vms_par_queue_peek(vms_par_queue *q, size_t n, void **ptr1, size_t *len1,
                          void **ptr2, size_t *len2) {
    size_t off;
    const size_t cur_elem_num = vms_spsc_ringbuf_peek(
        &q->ringbuf, n == 0 ? ~((size_t)0) : n, &off, len1, len2);
    if (__predict_true(cur_elem_num > 0)) {
        *ptr1 = q->data + (off * q->elem_size);

        if (__predict_false(*len2 > 0)) {
            *ptr2 = q->data;
        }
    }
    return cur_elem_num;
}

/* peak1 -- it is like top + return the number of elements */
size_t vms_par_queue_peek1(vms_par_queue *q, void **data) {
    size_t n;
    const size_t off = vms_spsc_ringbuf_read_off_nowrap(&q->ringbuf, &n);
    if (__predict_true(n > 0)) {
        *data = q->data + (off * q->elem_size);
    }
    return n;
}

vms_event *vms_par_queue_peek_at(vms_par_queue *q, size_t k) {
    unsigned char *ptr1, *ptr2;
    size_t len1, len2;

    size_t n = vms_par_queue_peek(q, k == (~((size_t)0)) ? k : k + 1,
                                  (void **)&ptr1, &len1, (void **)&ptr2, &len2);
    if (n <= k) {
        return NULL;
    }
    if (k < len1) {
        return (vms_event *)(ptr1 + (k * q->elem_size));
    }
    k -= len1;
    assert(k < len2);
    return (vms_event *)(ptr2 + (k * q->elem_size));
}

vms_event *vms_par_queue_peek_atmost_at(vms_par_queue *q, size_t *want_k) {
    void *ptr1, *ptr2;
    size_t len1, len2;
    size_t k = *want_k;

    size_t n = vms_par_queue_peek(q, k == (~((size_t)0)) ? k : k + 1, &ptr1,
                                  &len1, &ptr2, &len2);
    if (n == 0) {
        *want_k = 0;
        return 0;
    }
    if (n <= k) {
        k = n - 1;
        *want_k = k;
    }
    if (k < len1) {
        return (vms_event *)((unsigned char *)ptr1 + (k * q->elem_size));
    }
    k -= len1;
    assert(k < len2);
    return (vms_event *)((unsigned char *)ptr2 + (k * q->elem_size));
}
