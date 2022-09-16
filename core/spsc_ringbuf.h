#ifndef SHAMON_SPSC_RINGBUF_H
#define SHAMON_SPSC_RINGBUF_H

#include <stdbool.h>
#include <stdatomic.h>
#include <unistd.h>

/**
 * Single-producer single-consumer (SPSC) lock-free concurrent ring-buffer.
 */
typedef struct _shm_spsc_ringbuf {
    size_t capacity;
    /* modifications of this variable are going to be atomic,
     * but that is the all atomicity that we need! */
    _Atomic size_t elem_num;
    size_t head, tail;
#ifndef NDEBUG
    /* for checking the consistency of partial writes */
    size_t partial_head;
    size_t partial_write_n;
#endif
} shm_spsc_ringbuf;


void shm_spsc_ringbuf_init(shm_spsc_ringbuf *b, size_t capacity);
size_t shm_spsc_ringbuf_consume(shm_spsc_ringbuf *b, size_t n);
size_t shm_spsc_ringbuf_write_off(shm_spsc_ringbuf *b, size_t *n, size_t *wrap_n);
size_t shm_spsc_ringbuf_write_off_nowrap(shm_spsc_ringbuf *b, size_t *n);
void shm_spsc_ringbuf_write_finish(shm_spsc_ringbuf *q, size_t n);


size_t shm_spsc_ringbuf_size(shm_spsc_ringbuf *b);
size_t shm_spsc_ringbuf_capacity(shm_spsc_ringbuf *b);
size_t shm_spsc_ringbuf_free_num(shm_spsc_ringbuf *b);
bool shm_spsc_ringbuf_full(shm_spsc_ringbuf *b);

size_t shm_spsc_ringbuf_read_off_nowrap(shm_spsc_ringbuf *b, size_t *n);
void *shm_spsc_ringbuf_top(shm_spsc_ringbuf *b);
size_t shm_spsc_ringbuf_peek(shm_spsc_ringbuf *b, size_t n,
                             size_t *off, size_t *len1, size_t *len2);
#endif /* SHAMON_SPSC_RINGBUF_H */
