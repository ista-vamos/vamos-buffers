#ifndef SHAMON_QUEUE_SPSC_H
#define SHAMON_QUEUE_SPSC_H

#include <stdbool.h>
#include <unistd.h>

typedef struct _vms_event vms_event;

/**
 * vms_queue_spsc is a single-producer single-consumer (SPSC) lock-free
 * concurrent queue.
 */
typedef struct _vms_queue_spsc {
    size_t capacity;
    /* modifications of this variable are going to be atomic,
     * but that is the all atomicity that we need! */
    _Atomic size_t elem_num;
    size_t head, tail;
#ifndef NDEBUG
    /* for checking the consistency of writes */
    size_t partial_head;
    size_t writing_k_elems;
#endif
} vms_queue_spsc;

void vms_queue_spsc_init(vms_queue_spsc *q, size_t capacity);
void vms_queue_spsc_destroy(vms_queue_spsc *q);

/* ask to write `k` elements, returns the offset where to
   start writing and saves to `k` the actual number of elements
   that can be written continuously from that offset
   (i.e., if *k == 0, then the queue is empty)
size_t vms_queue_spsc_write_offsets(vms_queue_spsc *q, size_t k,
                                    size_t *len, size_t *wrapped_len);
                                    */
/* get next offset for writing the next element. \return true if
 * the write is possible, or false (which meanst that the queue is full) */
bool vms_queue_spsc_write_offset(vms_queue_spsc *q, size_t *offset);

/* notify queue that a write was finished and that `written_num`
 * offsets were used */
void vms_queue_spsc_writes_finish(vms_queue_spsc *q, size_t written_num);
/* notify queue that a single write was finished -- must match a single call of
 * vms_queue_spcs_write_offset (calls to vms_queue_spcs_write_offsets must be
 * "finished" by vms_queue_spcs_writes_finish */
void vms_queue_spsc_write_finish(vms_queue_spsc *q);

size_t vms_queue_spsc_consume(vms_queue_spsc *q, size_t k);

size_t vms_queue_spsc_capacity(vms_queue_spsc *q);
size_t vms_queue_spsc_size(vms_queue_spsc *q);
size_t vms_queue_spsc_free_num(vms_queue_spsc *q);

/* returns the number of elements in the queue. Note that it is not safe to
   access the whole range of offsets from [offset, offset+retval]
   as there can be a wrap-around. Either count with the possible wrap-around,
   or use vms_queue_spsc_peek() when reading more than one offset. */
size_t vms_queue_spsc_read_offset(vms_queue_spsc *q, size_t *offset);

size_t vms_queue_spsc_peek(vms_queue_spsc *q, size_t n, size_t *off1,
                           size_t *len1, size_t *off2, size_t *len2);

/* peek at a given element. k = 0 is the first elem (top), k = 1 the second, ...
 */
size_t vms_queue_spsc_peek_at(vms_queue_spsc *q, size_t k);
/* peek at a given element or the last possible readable element
 * if there is not enough elements. 'k' is set to the index (starting from 0)
 * of the peeked element. Returns the number of elements in the queue. */
size_t vms_queue_spsc_peek_atmost_at(vms_queue_spsc *q, size_t *k,
                                     size_t *offset);

#endif /* SHAMON_QUEUE_SPSC_H */
