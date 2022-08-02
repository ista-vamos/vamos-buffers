#include <stdlib.h>
#include <stdbool.h>
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

// allocate space for one element
// and return the pointer to this element.
// If there is no room, NULL is returned.
// The returned memory can be filled manually.
void *shm_queue_extend(shm_queue *q) {
    // queue is full
    if (q->elem_num == q->size)
        return NULL;

    // all ok, copy the data
    void *pos = q->data + q->head*q->elem_size;
    ++q->head;
    ++q->elem_num;

    // queue full, rotate it
    if (q->head == q->size) {
        q->head = 0;
    }

    return pos;
}


bool shm_queue_push(shm_queue *q, const void *elem) {
    void *pos = shm_queue_extend(q);
    if (pos) {
        memcpy(pos, elem, q->elem_size);
        return true;
    }
    return false;
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

size_t shm_queue_max_size(shm_queue *q) {
    return q->size;
}

size_t shm_queue_size(shm_queue *q) {
    return q->elem_num;
}
