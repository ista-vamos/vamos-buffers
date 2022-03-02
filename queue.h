#ifndef SHAMON_QUEUE_H
#define SHAMON_QUEUE_H

#include <unistd.h>
#include <stdbool.h>

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

#endif // SHAMON_QUEUE_H
