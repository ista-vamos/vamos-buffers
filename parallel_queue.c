#include <stdlib.h>
#include <stdbool.h>
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
}

void shm_par_queue_destroy(shm_par_queue *q) {
    free(q->data);
}

bool shm_par_queue_push(shm_par_queue *q, const void *elem) {
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

    memcpy(pos, elem, q->elem_size);

    // the increment must come after everything is done
    ++q->elem_num;

    return true;
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
    --q->elem_num;

    return true;
}

bool shm_par_queue_drop(shm_par_queue *q, size_t k) {
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

size_t shm_par_queue_capacity(shm_par_queue *q) {
    return q->capacity;
}

size_t shm_par_queue_size(shm_par_queue *q) {
    return q->elem_num;
}

size_t shm_par_queue_elem_size(shm_par_queue *q) {
    return q->elem_size;
}
