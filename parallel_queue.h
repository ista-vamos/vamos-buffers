#ifndef SHAMON_PARALLEL_QUEUE_H
#define SHAMON_PARALLEL_QUEUE_H

#include <unistd.h>
#include <stdbool.h>

typedef struct _shm_event shm_event;

typedef struct _shm_par_queue {
    size_t capacity;
    /* modifications of this variable are going to be atomic,
     * but that is the all atomicity that we need! */
    size_t elem_num;
    size_t elem_size;
    size_t head, tail;
#ifndef NDEBUG
    /* for checking the consistency of partial writes */
    size_t partial_head;
#endif
    unsigned char *data;
} shm_par_queue;

void shm_par_queue_init(shm_par_queue *q, size_t capacity, size_t elem_size);
void shm_par_queue_destroy(shm_par_queue *q);
bool shm_par_queue_push(shm_par_queue *q, const void *elem, size_t size);
size_t shm_par_queue_push_k(shm_par_queue *q, const void *elems, size_t k);
bool shm_par_queue_pop(shm_par_queue *q, void *buff);
size_t shm_par_queue_drop(shm_par_queue *q, size_t k);
size_t shm_par_queue_size(shm_par_queue *q);
size_t shm_par_queue_elem_size(shm_par_queue *q);
size_t shm_par_queue_capacity(shm_par_queue *q);
size_t shm_par_queue_free_num(shm_par_queue *q);
shm_event *shm_par_queue_top(shm_par_queue *q);
size_t shm_par_queue_peek(shm_par_queue *q, size_t n,
                          void **ptr1, size_t *len1,
                          void **ptr2, size_t *len2);

void *shm_par_queue_write_ptr(shm_par_queue *q);
bool shm_par_queue_write_finish(shm_par_queue *q);

#endif /* SHAMON_PARALLEL_QUEUE_H */
