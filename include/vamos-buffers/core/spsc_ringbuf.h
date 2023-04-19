#ifndef SHAMON_SPSC_RINGBUF_H
#define SHAMON_SPSC_RINGBUF_H

#include <stdatomic.h>
#include <stdbool.h>
#include <unistd.h>

#include "utils.h"

/**
 * Single-producer single-consumer (SPSC) lock-free concurrent ring-buffer.
 */
typedef struct _vms_spsc_ringbuf {
    /* capacity needs to be also aligned as it is read by both reader and writer
     */
    /* TODO: cache capacity locally (do a proxy to acces the ringbuf?
     * That would help also with using this ringbuf in shmbuf/) */
    CACHELINE_ALIGNED size_t capacity;

    /* reader */
    /* TODO: put these into a single cache line to save space
       or cache the seen head locally */
    CACHELINE_ALIGNED _Atomic size_t tail;
    CACHELINE_ALIGNED size_t seen_head;

    /* writer */
    /* TODO: put these into a single cache line to save space
       or cache the seen tail locally */
    CACHELINE_ALIGNED size_t seen_tail;
    CACHELINE_ALIGNED _Atomic size_t head;
#ifndef NDEBUG
    /* for checking the consistency of partial writes */
    CACHELINE_ALIGNED struct {
        size_t head;
        size_t n;
    } write_in_progress;
#endif
} vms_spsc_ringbuf;

void vms_spsc_ringbuf_init(vms_spsc_ringbuf *b, size_t capacity);
void vms_spsc_ringbuf_consume(vms_spsc_ringbuf *b, size_t n);
size_t vms_spsc_ringbuf_consume_upto(vms_spsc_ringbuf *b, size_t n);
size_t vms_spsc_ringbuf_write_off(vms_spsc_ringbuf *b, size_t *n,
                                  size_t *wrap_n);
size_t vms_spsc_ringbuf_write_off_nowrap(vms_spsc_ringbuf *b, size_t *n);
size_t vms_spsc_ringbuf_acquire(vms_spsc_ringbuf *b, size_t *n, size_t *wrap);
size_t vms_spsc_ringbuf_acquire_nowrap(vms_spsc_ringbuf *b, size_t *n);
void vms_spsc_ringbuf_write_finish(vms_spsc_ringbuf *q, size_t n);

size_t vms_spsc_ringbuf_size(vms_spsc_ringbuf *b);
size_t vms_spsc_ringbuf_max_size(vms_spsc_ringbuf *b);
/* capacity == max_size + 1, because we use one element as a separator.
   Capacity - 1 is the maximal offset that the ringbuf considers. */
size_t vms_spsc_ringbuf_capacity(vms_spsc_ringbuf *b);
size_t vms_spsc_ringbuf_free_num(vms_spsc_ringbuf *b);
bool vms_spsc_ringbuf_full(vms_spsc_ringbuf *b);
bool vms_spsc_ringbuf_empty(vms_spsc_ringbuf *b);

size_t vms_spsc_ringbuf_read_off_nowrap(vms_spsc_ringbuf *b, size_t *n);
size_t vms_spsc_ringbuf_read_acquire(vms_spsc_ringbuf *b, size_t *n);
void *vms_spsc_ringbuf_top(vms_spsc_ringbuf *b);
size_t vms_spsc_ringbuf_peek(vms_spsc_ringbuf *b, size_t n, size_t *off,
                             size_t *len1, size_t *len2);
#endif /* SHAMON_SPSC_RINGBUF_H */
