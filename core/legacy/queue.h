#ifndef SHAMON_QUEUE_H
#define SHAMON_QUEUE_H

#include <stdbool.h>
#include <unistd.h>

typedef struct _shm_queue {
    size_t size;
    size_t elem_num;
    size_t elem_size;
    size_t head, tail;
    unsigned char *data;
} shm_queue;

void shm_queue_init(shm_queue *q, size_t size, size_t elem_size);
void shm_queue_destroy(shm_queue *q);
bool shm_queue_push(shm_queue *q, const void *elem);
bool shm_queue_pop(shm_queue *q, void *buff);
size_t shm_queue_size(shm_queue *q);
size_t shm_queue_max_size(shm_queue *q);
// allocate space for one element
// and return the pointer to this element.
// If there is no room, NULL is returned.
// The returned memory can be filled manually.
void *shm_queue_extend(shm_queue *q);

#endif  // SHAMON_QUEUE_H
