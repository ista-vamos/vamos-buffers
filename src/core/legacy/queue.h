#ifndef VAMOS_QUEUE_H
#define VAMOS_QUEUE_H

#include <stdbool.h>
#include <unistd.h>

typedef struct _vms_queue {
    size_t size;
    size_t elem_num;
    size_t elem_size;
    size_t head, tail;
    unsigned char *data;
} vms_queue;

void vms_queue_init(vms_queue *q, size_t size, size_t elem_size);
void vms_queue_destroy(vms_queue *q);
bool vms_queue_push(vms_queue *q, const void *elem);
bool vms_queue_pop(vms_queue *q, void *buff);
size_t vms_queue_size(vms_queue *q);
size_t vms_queue_max_size(vms_queue *q);
// allocate space for one element
// and return the pointer to this element.
// If there is no room, NULL is returned.
// The returned memory can be filled manually.
void *vms_queue_extend(vms_queue *q);

#endif  // VAMOS_QUEUE_H
