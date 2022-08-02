#ifndef SHAMON_PARALLEL_QUEUE_H
#define SHAMON_PARALLEL_QUEUE_H

#include <unistd.h>
#include <stdbool.h>

typedef struct _shm_event shm_event;


typedef struct _shm_par_queue {
    size_t capacity;
    // this is all the atomicity we need!
    _Atomic size_t elem_num;
    size_t elem_size;
    size_t head, tail;
    size_t write_order_head;
#ifndef NDEBUG
    /* for checking the consistency of partial writes */
    size_t partial_head;
#endif
    unsigned char *data;
} shm_par_queue_mw;

void shm_par_queue_mw_init(shm_par_queue_mw *q, size_t capacity, size_t elem_size);
void shm_par_queue_mw_destroy(shm_par_queue_mw *q);
bool shm_par_queue_mw_push(shm_par_queue_mw *q, const void *elem, size_t size);
size_t shm_par_queue_mw_push_k(shm_par_queue_mw *q, const void *elems, size_t k);
bool shm_par_queue_mw_pop(shm_par_queue_mw *q, void *buff);
bool shm_par_queue_mw_drop(shm_par_queue_mw *q, size_t k);
size_t shm_par_queue_mw_size(shm_par_queue_mw *q);
size_t shm_par_queue_mw_elem_size(shm_par_queue_mw *q);
size_t shm_par_queue_mw_capacity(shm_par_queue_mw *q);
size_t shm_par_queue_mw_free_num(shm_par_queue_mw *q);
shm_event *shm_par_queue_mw_top(shm_par_queue_mw *q);
size_t shm_par_queue_mw_peek(shm_par_queue_mw *q, size_t n,
                          void **ptr1, size_t *len1,
                          void **ptr2, size_t *len2);

void *shm_par_queue_mw_write_ptr(shm_par_queue_mw *q);
bool shm_par_queue_mw_write_finish(shm_par_queue_mw *q);

#endif /* SHAMON_PARALLEL_QUEUE_H */
