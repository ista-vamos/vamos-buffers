#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "vamos-buffers/core/queue_spsc.h"

#define __predict_false(x) __builtin_expect((x) != 0, 0)
#define __predict_true(x) __builtin_expect((x) != 0, 1)

static inline void elem_num_inc(vms_queue_spsc *q, int k) {
    /* Do ++q->elem_num atomically. */
    /* The increment must come after everything is done.
       The release order makes sure that the written element
       is visible to other threads by now. */
    atomic_fetch_add_explicit(&q->elem_num, k, memory_order_release);
}

static inline void elem_num_dec(vms_queue_spsc *q, int k) {
    /* Do q->elem_num -= k atomically. */
    atomic_fetch_sub_explicit(&q->elem_num, k, memory_order_acquire);
}

static inline size_t elem_num(vms_queue_spsc *q) {
    return atomic_load_explicit(&q->elem_num, memory_order_relaxed);
}

void vms_queue_spsc_init(vms_queue_spsc *q, size_t capacity) {
    assert(q);
    assert(capacity > 0);
    q->elem_num = 0;
    q->capacity = capacity;
    q->head = q->tail = 0;
}

void vms_queue_spsc_destroy(vms_queue_spsc *q) {
    q->capacity = 0;
    q->elem_num = 0;
}

bool vms_queue_spsc_write_offset(vms_queue_spsc *q, size_t *offset) {
    if (__predict_false(elem_num(q) == q->capacity)) {
        return false;
    }

    *offset = q->head;

#ifndef NDEBUG
    q->partial_head = q->head;
    q->writing_k_elems = 1;
#endif

    return true;
}

void vms_queue_spsc_writes_finish(vms_queue_spsc *q, size_t written_num) {
    assert(q->partial_head == q->head && "Inconsistent head");
    assert(written_num <= q->writing_k_elems);
    assert(q->head + written_num <= q->capacity);

    q->head += written_num;

    /* queue is full, rotate it */
    if (__predict_false(q->head == q->capacity)) {
        q->head = 0;
    }

    elem_num_inc(q, written_num);
}

void vms_queue_spsc_write_finish(vms_queue_spsc *q) {
    vms_queue_spsc_writes_finish(q, 1);
}

size_t vms_queue_spsc_consume(vms_queue_spsc *q, size_t k) {
    assert(k > 0);
    size_t num = elem_num(q);
    if (__predict_false(num < k)) {
        k = num;
    }

    q->tail += k;
    if (__predict_false(q->tail >= q->capacity))
        q->tail -= q->capacity;

    assert(elem_num(q) >= k);
    elem_num_dec(q, k);

    return k;
}

size_t vms_queue_spsc_free_num(vms_queue_spsc *q) {
    return q->capacity - elem_num(q);
}

size_t vms_queue_spsc_capacity(vms_queue_spsc *q) { return q->capacity; }

size_t vms_queue_spsc_size(vms_queue_spsc *q) { return elem_num(q); }

size_t vms_queue_spsc_peek(vms_queue_spsc *q, size_t n, size_t *off1,
                           size_t *len1, size_t *off2, size_t *len2) {
    size_t cur_elem_num = elem_num(q);
    if (n > cur_elem_num)
        n = cur_elem_num;
    size_t end = n + q->tail;
    if (end >= q->capacity) {
        *off1 = q->tail;
        *len1 = q->capacity - q->tail;
        *off2 = 0;
        *len2 = end - q->capacity;
    } else {
        *off1 = q->tail;
        *len1 = n;
        *len2 = 0;
    }
    assert(*len1 + *len2 == n);
    /* return  n; */
    return cur_elem_num;
}

/* peak1 -- it is like top + return the number of elements */
size_t vms_queue_spsc_read_offset(vms_queue_spsc *q, size_t *offset) {
    *offset = q->tail;
    return elem_num(q);
}

/*
size_t vms_queue_spsc_peek_at(vms_queue_spsc *q, size_t idx) {
    size_t cur_elem_num = elem_num(q);
    if (idx >= cur_elem_num)
        return NULL;

    size_t end = k + q->tail;
    if (end >= q->capacity) {
        return (vms_event *)(q->data + (end - q->capacity) * q->elem_size);
    }

    return (vms_event *)(q->data + (q->tail + k) * q->elem_size);
}
*/

size_t vms_queue_spsc_peek_atmost_at(vms_queue_spsc *q, size_t *want_k,
                                     size_t *offset) {
    size_t num = elem_num(q);
    if (__predict_false(num == 0))
        return 0;

    if (*want_k >= num) {
        /* k is the number of elements, so the index is one less */
        *want_k = --num;
    }

    size_t end = *want_k + q->tail;
    if (end >= q->capacity) {
        *offset = end - q->capacity;
    } else {
        *offset = end;
    }

    return num;
}
