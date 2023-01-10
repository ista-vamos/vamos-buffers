#ifndef SHAMON_PARALLEL_QUEUE_H
#define SHAMON_PARALLEL_QUEUE_H

#include <stdbool.h>
#include <unistd.h>

#include "spsc_ringbuf.h"

typedef struct _shm_event shm_event;

/**
 * shm_par_queue is a single-producer single-consumer (SPSC) lock-free
 * concurrent queue.
 */
typedef struct _shm_par_queue {
    shm_spsc_ringbuf ringbuf;
    size_t elem_size;
    size_t capacity;
    unsigned char *data;

    char __padding[CACHELINE_SIZE - 2 * sizeof(size_t) -
                   sizeof(unsigned char *)];
} shm_par_queue;

void shm_par_queue_init(shm_par_queue *q, size_t capacity, size_t elem_size);
void shm_par_queue_destroy(shm_par_queue *q);
bool shm_par_queue_push(shm_par_queue *q, const void *elem, size_t size);
bool shm_par_queue_pop(shm_par_queue *q, void *buff);
void shm_par_queue_drop(shm_par_queue *q, size_t k);
size_t shm_par_queue_size(shm_par_queue *q);
size_t shm_par_queue_elem_size(shm_par_queue *q);
size_t shm_par_queue_capacity(shm_par_queue *q);
size_t shm_par_queue_free_num(shm_par_queue *q);
shm_event *shm_par_queue_top(shm_par_queue *q);
size_t shm_par_queue_peek(shm_par_queue *q, size_t n, void **ptr1, size_t *len1,
                          void **ptr2, size_t *len2);
/* peek 1 event */
size_t shm_par_queue_peek1(shm_par_queue *q, void **data);

/* peek at a given element. k = 0 is the first elem (top), k = 1 the second, ...
 */
shm_event *shm_par_queue_peek_at(shm_par_queue *q, size_t k);
/* peek at a given element or the last possible readable element
 * if there is not enough elements. 'k' is set to the index (starting from 0)
 * of the peeked element. Returns NULL only if the queue is empty. */
shm_event *shm_par_queue_peek_atmost_at(shm_par_queue *q, size_t *k);

void *shm_par_queue_write_ptr(shm_par_queue *q);
void shm_par_queue_write_finish(shm_par_queue *q);

#endif /* SHAMON_PARALLEL_QUEUE_H */
