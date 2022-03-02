#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "queue.h"


void shm_queue_init(shm_queue *q, size_t size, size_t elem_size) {
    assert(q);
    assert(size > 0);
    assert(elem_size > 0);
    q->elem_num = 0;
    q->size = size;
    q->elem_size = elem_size;
    q->head = q->tail = 0;
    q->data = malloc(size * elem_size);
}

void shm_queue_destroy(shm_queue *q) {
    free(q->data);
}

bool shm_queue_push(shm_queue *q, const void *elem) {
    // queue is full
    if (q->elem_num == q->size)
        return false;

    // all ok, copy the data
    unsigned char *pos = q->data + q->head*q->elem_size;
    memcpy(pos, elem, q->elem_size);
    ++q->head;
    ++q->elem_num;

    // queue full, rotate it
    if (q->head == q->size) {
        q->head = 0;
    }
    return true;
}

bool shm_queue_pop(shm_queue *q, void *buff) {
    if (q->elem_num == 0) {
        return false;
    }

    unsigned char *pos = q->data + q->tail*q->elem_size;
    memcpy(buff, pos, q->elem_size);
    ++q->tail;
    --q->elem_num;
    if (q->tail == q->size)
        q->tail = 0;
    return true;
}


