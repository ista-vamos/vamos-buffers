#ifndef SHAMON_PARALLEL_QUEUE_H
#define SHAMON_PARALLEL_QUEUE_H

#include <unistd.h>
#include <stdbool.h>

typedef struct _shm_par_queue {
    size_t capacity;
    // this is all the atomicity we need!
    _Atomic size_t elem_num;
    size_t elem_size;
    size_t head, tail;
    unsigned char *data;
} shm_par_queue;

void shm_par_queue_init(shm_par_queue *q, size_t capacity, size_t elem_size);
void shm_par_queue_destroy(shm_par_queue *q);
bool shm_par_queue_push(shm_par_queue *q, const void *elem);
bool shm_par_queue_pop(shm_par_queue *q, void *buff);
size_t shm_par_queue_size(shm_par_queue *q);
size_t shm_par_queue_capacity(shm_par_queue *q);

#endif /* SHAMON_PARALLEL_QUEUE_H */
