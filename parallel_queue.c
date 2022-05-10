#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <string.h>
#include <assert.h>
#include "parallel_queue.h"


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
bool shm_par_queue_push(shm_par_queue *q,
                        const void *elem,
                        size_t size) {
    // queue is full
    if (q->elem_num == q->capacity) {
        return false;
    }

    // all ok, copy the data
    void *pos = q->data + q->head*q->elem_size;
    ++q->head;

    // queue full, rotate it
    if (q->head == q->capacity) {
        q->head = 0;
    }

    assert(q->elem_size >= size && "Size does not fit the slot");
    memcpy(pos, elem, size);

    /* Do ++info->elem_num atomically. */
    /* The increment must come after everything is done.
       The release order makes sure that the written element
       is visible to other threads by now. */
    atomic_fetch_add_explicit(&q->elem_num, 1,
                              memory_order_release);

    return true;
}

void *shm_par_queue_write_ptr(shm_par_queue *q) {
    /* queue is full */
    if (q->elem_num == q->capacity) {
        return NULL;
    }
#ifndef NDEBUG
    q->partial_head = q->head;
#endif

    // all ok, copy the data
    return q->data + q->head*q->elem_size;
}

bool shm_par_queue_write_finish(shm_par_queue *q) {
    assert(q->partial_head == q->head
           && "Inconsistent head, was push() called?");
    ++q->head;

    // queue full, rotate it
    if (q->head == q->capacity) {
        q->head = 0;
    }

    /* Do ++info->elem_num atomically. */
    /* The increment must come after everything is done.
       The release order makes sure that the written element
       is visible to other threads by now. */
    atomic_fetch_add_explicit(&q->elem_num, 1,
                              memory_order_release);

    return true;
}

/* return how many elements were not pushed */
size_t shm_par_queue_push_k(shm_par_queue *q,
                            const void *elems,
                            size_t k) {
    /* how many elements can we actually fit in? */
    size_t freen = q->capacity - q->elem_num;
    size_t ret = 0;
    if (freen < k) {
        ret = k - freen;
        k = freen;
    }

    void *pos = q->data + q->head*q->elem_size;
    size_t end = q->head + k;
    if (end > q->capacity) {
        size_t ovfl = end - q->capacity;
        memcpy(pos, elems, q->elem_size*(k - ovfl));
        memcpy(q->data,
               elems + q->elem_size*(k-ovfl),
               q->elem_size*ovfl);
        q->head = ovfl;
    } else {
        assert(k <= q->capacity && "Too many elements to push");
        memcpy(pos, elems, q->elem_size*k);
        q->head += k;
        /* queue full, rotate it */
        assert(q->head <= q->capacity);
        if (q->head == q->capacity) {
            q->head = 0;
        }
    }

    /* Do info->elem_num += k atomically.
     * The increment must come after everything is done.
       The release order makes sure that the written element
       is visible to other threads by now. */
    atomic_fetch_add_explicit(&q->elem_num, k,
                              memory_order_release);

    return ret;
}


bool shm_par_queue_pop(shm_par_queue *q, void *buff) {
    if (q->elem_num == 0) {
        return false;
    }

    unsigned char *pos = q->data + q->tail*q->elem_size;
    memcpy(buff, pos, q->elem_size);
    ++q->tail;
    if (q->tail == q->capacity)
        q->tail = 0;

    assert(q->elem_num > 0);
    /* Do  --info->elem_num atomically */
    atomic_fetch_sub_explicit(&q->elem_num, 1,
                              memory_order_relaxed);

    return true;
}

size_t shm_par_queue_drop(shm_par_queue *q, size_t k) {
    if (q->elem_num < k) {
        k = q->elem_num;
    }

    q->tail += k;
    if (q->tail >= q->capacity)
        q->tail -= q->capacity;

    assert(q->elem_num >= k);
    /* Do info->elem_num -= k atomically. */
    atomic_fetch_sub_explicit(&q->elem_num, k,
                              memory_order_relaxed);

    return k;
}

size_t shm_par_queue_free_num(shm_par_queue *q) {
    return q->capacity - q->elem_num;
}

size_t shm_par_queue_capacity(shm_par_queue *q) {
    return q->capacity;
}

size_t shm_par_queue_size(shm_par_queue *q) {
    return q->elem_num;
}

size_t shm_par_queue_elem_size(shm_par_queue *q) {
    return q->elem_size;
}

shm_event *shm_par_queue_top(shm_par_queue *q) {
    if (q->elem_num > 0)
        return (shm_event *)(q->data + q->tail*q->elem_size);
    return (shm_event *)0;
}
size_t shm_par_queue_peek(shm_par_queue *q, size_t n,
                          void **ptr1, size_t *len1,
                          void **ptr2, size_t *len2) {
    size_t cur_elem_num = q->elem_num;
    if (n > cur_elem_num)
        n = cur_elem_num;
    size_t end = cur_elem_num + q->tail;
    if (end > q->capacity) {
        *ptr1 = q->data + q->tail*q->elem_size;
        *len1 = q->capacity - q->tail;
        *ptr2 = q->data;
        *len2 = end - q->capacity;
    } else {
        *ptr1 = q->data + q->tail*q->elem_size;
        *len1 = n;
        *len2 = 0;
    }
    assert(*len1 + *len2 == n);
    return  n;
}

shm_event *shm_par_queue_peek_at(shm_par_queue *q, size_t k) {
    size_t cur_elem_num = q->elem_num;
    if (k >= cur_elem_num)
        return NULL;

    size_t end = k + q->tail;
    if (end > q->capacity) {
        return (shm_event*)(q->data + (end - q->capacity)*q->elem_size);
    }

    return (shm_event*)(q->data + (q->tail + k)*q->elem_size);
}
