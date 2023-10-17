#ifndef VAMOS_PARALLEL_QUEUE_H
#define VAMOS_PARALLEL_QUEUE_H

#include <stdbool.h>
#include <unistd.h>

#include "spsc_ringbuf.h"

typedef struct _vms_event vms_event;

/**
 * vms_par_queue is a single-producer single-consumer (SPSC) lock-free
 * concurrent queue.
 */
typedef struct _vms_par_queue {
    vms_spsc_ringbuf ringbuf;
    size_t elem_size;
    size_t capacity;
    unsigned char *data;

    char __padding[CACHELINE_SIZE - 2 * sizeof(size_t) -
                   sizeof(unsigned char *)];
} vms_par_queue;

void vms_par_queue_init(vms_par_queue *q, size_t capacity, size_t elem_size);
void vms_par_queue_destroy(vms_par_queue *q);
bool vms_par_queue_push(vms_par_queue *q, const void *elem, size_t size);
bool vms_par_queue_pop(vms_par_queue *q, void *buff);
void vms_par_queue_drop(vms_par_queue *q, size_t k);
size_t vms_par_queue_size(vms_par_queue *q);
size_t vms_par_queue_elem_size(vms_par_queue *q);
size_t vms_par_queue_capacity(vms_par_queue *q);
size_t vms_par_queue_free_num(vms_par_queue *q);
vms_event *vms_par_queue_top(vms_par_queue *q);
size_t vms_par_queue_peek(vms_par_queue *q, size_t n, void **ptr1, size_t *len1,
                          void **ptr2, size_t *len2);
/* peek 1 event */
size_t vms_par_queue_peek1(vms_par_queue *q, void **data);

/* peek at a given element. k = 0 is the first elem (top), k = 1 the second, ...
 */
vms_event *vms_par_queue_peek_at(vms_par_queue *q, size_t k);
/* peek at a given element or the last possible readable element
 * if there is not enough elements. 'k' is set to the index (starting from 0)
 * of the peeked element. Returns NULL only if the queue is empty. */
vms_event *vms_par_queue_peek_atmost_at(vms_par_queue *q, size_t *k);

void *vms_par_queue_write_ptr(vms_par_queue *q);
void vms_par_queue_write_finish(vms_par_queue *q);

#endif /* VAMOS_PARALLEL_QUEUE_H */
