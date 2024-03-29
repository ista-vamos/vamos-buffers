#include "queue.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void vms_queue_init(vms_queue *q, size_t size, size_t elem_size) {
    assert(q);
    assert(size > 0);
    assert(elem_size > 0);
    q->elem_num = 0;
    q->size = size;
    q->elem_size = elem_size;
    q->head = q->tail = 0;
    q->data = malloc(size * elem_size);
}

void vms_queue_destroy(vms_queue *q) { free(q->data); }

// allocate space for one element
// and return the pointer to this element.
// If there is no room, NULL is returned.
// The returned memory can be filled manually.
void *vms_queue_extend(vms_queue *q) {
    // queue is full
    if (q->elem_num == q->size)
        return NULL;

    // all ok, copy the data
    void *pos = q->data + q->head * q->elem_size;
    ++q->head;
    ++q->elem_num;

    // queue full, rotate it
    if (q->head == q->size) {
        q->head = 0;
    }

    return pos;
}

bool vms_queue_push(vms_queue *q, const void *elem) {
    void *pos = vms_queue_extend(q);
    if (pos) {
        memcpy(pos, elem, q->elem_size);
        return true;
    }
    return false;
}

bool vms_queue_pop(vms_queue *q, void *buff) {
    if (q->elem_num == 0) {
        return false;
    }

    unsigned char *pos = q->data + q->tail * q->elem_size;
    memcpy(buff, pos, q->elem_size);
    ++q->tail;
    --q->elem_num;
    if (q->tail == q->size)
        q->tail = 0;
    return true;
}

size_t vms_queue_max_size(vms_queue *q) { return q->size; }

size_t vms_queue_size(vms_queue *q) { return q->elem_num; }
