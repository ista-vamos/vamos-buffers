#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <immintrin.h> /* _mm_pause */

#include "par_queue_mw.h"

void shm_par_queue_mw_init(shm_par_queue_mw *q, size_t capacity,
                           size_t elem_size) {
    assert(q);
    assert(capacity > 0);
    assert(elem_size > 0);
    q->elem_num = 0;
    q->capacity = capacity;
    q->elem_size = elem_size;
    q->head = q->tail = 0;
    q->write_order_head = 0;
    q->data = malloc(capacity * elem_size);
    assert(q->data && "Allocation failed");
}

void shm_par_queue_mw_destroy(shm_par_queue_mw *q) {
    free(q->data);
}

/* push an element into the queue.
 * 'size' is the actual size of the pushed element
 * and it must hold that 'size' <= 'elem_size' */
bool shm_par_queue_mw_push(shm_par_queue_mw *q, const void *elem, size_t size) {
    // queue is full
    if (q->elem_num == q->capacity) {
        return false;
    }

    // all ok, copy the data
    /* atomically shift the head */
    size_t head = q->head, newhead;
    do {
        assert(head < q->capacity && "Head can never be the capacity");
        newhead = head + 1;
        if (newhead == q->capacity)
            newhead = 0;
        /* TODO: use explicit memory ordering */
    } while (atomic_compare_exchange_weak(&q->head, &head, newhead));

    void *pos = q->data + head * q->elem_size;

    assert(q->elem_size >= size && "Size does not fit the slot");
    memcpy(pos, elem, size);

    while (q->write_order_head != head)
        _mm_pause();

    /* update the write_order_head */
    head = q->write_order_head;
    do {
        assert(head < q->capacity && "Head can never be the capacity");
        newhead = head + 1;
        if (newhead == q->capacity)
            newhead = 0;
        /* TODO: use explicit memory ordering */
    } while (
        atomic_compare_exchange_weak(&q->write_order_head, &head, newhead));

    // the increment must come after everything is done
    ++q->elem_num;

    return true;
}

void *shm_par_queue_mw_write_ptr(shm_par_queue_mw *q) {
    /* queue is full */
    if (q->elem_num == q->capacity) {
        return NULL;
    }
#ifndef NDEBUG
    q->partial_head = q->head;
#endif

    // all ok, copy the data
    return q->data + q->head * q->elem_size;
}

bool shm_par_queue_mw_write_finish(shm_par_queue_mw *q) {
    assert(q->partial_head == q->head &&
           "Inconsistent head, was push() called?");
    ++q->head;

    // queue full, rotate it
    if (q->head == q->capacity) {
        q->head = 0;
    }

    // the increment must come after everything is done
    ++q->elem_num;

    return true;
}

/* return how many elements were not pushed */
size_t shm_par_queue_mw_push_k(shm_par_queue_mw *q, const void *elems,
                               size_t k) {
    /* how many elements can we actually fit in? */
    size_t freen = q->capacity - q->elem_num;
    size_t ret = 0;
    if (freen < k) {
        ret = k - freen;
        k = freen;
    }

    void *pos = q->data + q->head * q->elem_size;
    size_t end = q->head + k;
    if (end > q->capacity) {
        size_t ovfl = end - q->capacity;
        memcpy(pos, elems, q->elem_size * (k - ovfl));
        memcpy(q->data, elems + q->elem_size * (k - ovfl), q->elem_size * ovfl);
        q->head = ovfl;
    } else {
        assert(k <= q->capacity && "Too many elements to push");
        memcpy(pos, elems, q->elem_size * k);
        q->head += k;
        /* queue full, rotate it */
        assert(q->head <= q->capacity);
        if (q->head == q->capacity) {
            q->head = 0;
        }
    }

    q->elem_num += k;

    return ret;
}

bool shm_par_queue_mw_pop(shm_par_queue_mw *q, void *buff) {
    if (q->elem_num == 0) {
        return false;
    }

    unsigned char *pos = q->data + q->tail * q->elem_size;
    memcpy(buff, pos, q->elem_size);
    ++q->tail;
    if (q->tail == q->capacity)
        q->tail = 0;

    assert(q->elem_num > 0);
    --q->elem_num;

    return true;
}

bool shm_par_queue_mw_drop(shm_par_queue_mw *q, size_t k) {
    if (k > q->capacity)
        k = q->capacity;

    if (q->elem_num < k) {
        return false;
    }

    q->tail += k;
    if (q->tail >= q->capacity)
        q->tail -= q->capacity;

    assert(q->elem_num >= k);
    q->elem_num -= k;

    return true;
}

size_t shm_par_queue_mw_free_num(shm_par_queue_mw *q) {
    return q->capacity - q->elem_num;
}

size_t shm_par_queue_mw_capacity(shm_par_queue_mw *q) {
    return q->capacity;
}

size_t shm_par_queue_mw_size(shm_par_queue_mw *q) {
    return q->elem_num;
}

size_t shm_par_queue_mw_elem_size(shm_par_queue_mw *q) {
    return q->elem_size;
}

shm_event *shm_par_queue_mw_top(shm_par_queue_mw *q) {
    if (q->elem_num > 0)
        return (shm_event *)(q->data + q->tail * q->elem_size);
    return (shm_event *)0;
}
size_t shm_par_queue_mw_peek(shm_par_queue_mw *q, size_t n, void **ptr1,
                             size_t *len1, void **ptr2, size_t *len2) {
    size_t cur_elem_num = q->elem_num;
    if (n > cur_elem_num)
        n = cur_elem_num;
    size_t end = cur_elem_num + q->tail;
    if (end > q->capacity) {
        *ptr1 = q->data + q->tail * q->elem_size;
        *len1 = q->capacity - q->tail;
        *ptr2 = q->data;
        *len2 = end - q->capacity;
    } else {
        *ptr1 = q->data + q->tail * q->elem_size;
        *len1 = n;
        *len2 = 0;
    }
    assert(*len1 + *len2 == n);
    return n;
}
