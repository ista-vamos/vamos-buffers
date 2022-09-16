#include "par_queue.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static inline void elem_num_inc(shm_par_queue *q, int k) {
    /* Do ++q->elem_num atomically. */
    /* The increment must come after everything is done.
       The release order makes sure that the written element
       is visible to other threads by now. */
    atomic_fetch_add_explicit(&q->elem_num, k, memory_order_seq_cst);
}

static inline void elem_num_dec(shm_par_queue *q, int k) {
    /* Do q->elem_num -= k atomically. */
    atomic_fetch_sub_explicit(&q->elem_num, k, memory_order_seq_cst);
}

static inline size_t elem_num(shm_par_queue *q) {
    return atomic_load_explicit(&q->elem_num, memory_order_seq_cst);
}

void shm_par_queue_init(shm_par_queue *q, size_t capacity, size_t elem_size) {
    assert(q);
    assert(capacity > 0);
    assert(elem_size > 0);
    q->elem_num = 0;
    q->capacity = capacity;
    q->elem_size = elem_size;
    q->head = q->tail = 0;
    q->data = malloc(capacity * elem_size);
    assert(q->data && "Allocation failed");
}

void shm_par_queue_destroy(shm_par_queue *q) {
    free(q->data);
}

/* push an element into the queue.
 * 'size' is the actual size of the pushed element
 * and it must hold that 'size' <= 'elem_size' */
bool shm_par_queue_push(shm_par_queue *q, const void *elem, size_t size) {
    // queue is full
    if (elem_num(q) == q->capacity) {
        return false;
    }

    // all ok, copy the data
    void *pos = q->data + q->head * q->elem_size;
    ++q->head;

    // queue full, rotate it
    if (q->head == q->capacity) {
        q->head = 0;
    }

    assert(q->elem_size >= size && "Size does not fit the slot");
    memcpy(pos, elem, size);

    elem_num_inc(q, 1);

    return true;
}

void *shm_par_queue_write_ptr(shm_par_queue *q) {
    /* queue is full */
    if (elem_num(q) == q->capacity) {
        return NULL;
    }
#ifndef NDEBUG
    q->partial_head = q->head;
#endif

    // all ok, copy the data
    return q->data + q->head * q->elem_size;
}

bool shm_par_queue_write_finish(shm_par_queue *q) {
    assert(q->partial_head == q->head &&
           "Inconsistent head, was push() called?");
    ++q->head;

    // queue full, rotate it
    if (q->head == q->capacity) {
        q->head = 0;
    }

    elem_num_inc(q, 1);

    return true;
}

bool shm_par_queue_pop(shm_par_queue *q, void *buff) {
    if (elem_num(q) == 0) {
        return false;
    }

    unsigned char *pos = q->data + q->tail * q->elem_size;
    memcpy(buff, pos, q->elem_size);
    ++q->tail;
    if (q->tail == q->capacity)
        q->tail = 0;

    assert(elem_num(q) > 0);
    elem_num_dec(q, 1);

    return true;
}

size_t shm_par_queue_drop(shm_par_queue *q, size_t k) {
    assert(k > 0);
    size_t num = elem_num(q);
    if (num < k) {
        k = num;
    }

    q->tail += k;
    if (q->tail >= q->capacity)
        q->tail -= q->capacity;

    assert(elem_num(q) >= k);
    elem_num_dec(q, k);

    return k;
}

size_t shm_par_queue_free_num(shm_par_queue *q) {
    return q->capacity - elem_num(q);
}

size_t shm_par_queue_capacity(shm_par_queue *q) {
    return q->capacity;
}

size_t shm_par_queue_size(shm_par_queue *q) {
    return elem_num(q);
}

size_t shm_par_queue_elem_size(shm_par_queue *q) {
    return q->elem_size;
}

shm_event *shm_par_queue_top(shm_par_queue *q) {
    if (elem_num(q) > 0)
        return (shm_event *)(q->data + q->tail * q->elem_size);
    return (shm_event *)0;
}

size_t shm_par_queue_peek(shm_par_queue *q, size_t n, void **ptr1, size_t *len1,
                          void **ptr2, size_t *len2) {
    size_t cur_elem_num = elem_num(q);
    if (n > cur_elem_num)
        n = cur_elem_num;
    size_t end = n + q->tail;
    if (end >= q->capacity) {
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
    /* return  n; */
    return cur_elem_num;
}

/* peak1 -- it is like top + return the number of elements */
size_t shm_par_queue_peek1(shm_par_queue *q, void **data) {
    size_t cur_elem_num = elem_num(q);
    if (cur_elem_num > 0) {
        *data = (unsigned char *)(q->data + q->tail * q->elem_size);
    }
    return cur_elem_num;
}

shm_event *shm_par_queue_peek_at(shm_par_queue *q, size_t k) {
    size_t cur_elem_num = elem_num(q);
    if (k >= cur_elem_num)
        return NULL;

    size_t end = k + q->tail;
    if (end >= q->capacity) {
        return (shm_event *)(q->data + (end - q->capacity) * q->elem_size);
    }

    return (shm_event *)(q->data + (q->tail + k) * q->elem_size);
}

shm_event *shm_par_queue_peek_atmost_at(shm_par_queue *q, size_t *want_k) {
    size_t k = elem_num(q);
    if (k == 0)
        return NULL;
    if (*want_k >= k) {
        /* k is the number of elements, so the index is one less */
        *want_k = --k;
    } else {
        k = *want_k;
    }

    size_t end = k + q->tail;
    if (end >= q->capacity) {
        return (shm_event *)(q->data + (end - q->capacity) * q->elem_size);
    }

    return (shm_event *)(q->data + (q->tail + k) * q->elem_size);
}
