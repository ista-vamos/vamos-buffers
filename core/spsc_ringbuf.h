#ifndef SHAMON_SPSC_RINGBUF_H
#define SHAMON_SPSC_RINGBUF_H

#include <stdbool.h>
#include <stdatomic.h>
#include <unistd.h>

/* On x86 and ARM the cache line has 64 bytes,
   change if needed. */
#define CACHELINE_SIZE 64
#define CACHELINE_ALIGNED _Alignas(CACHELINE_SIZE)

/**
 * Single-producer single-consumer (SPSC) lock-free concurrent ring-buffer.
 */
typedef struct _shm_spsc_ringbuf {
    /* capacity needs to be also aligned as it is read by both reader and writer */
    CACHELINE_ALIGNED size_t capacity;

    /* reader */
    CACHELINE_ALIGNED _Atomic size_t tail;
    CACHELINE_ALIGNED size_t seen_head;

    /* writer */
    CACHELINE_ALIGNED size_t seen_tail;
    CACHELINE_ALIGNED _Atomic size_t head;
#ifndef NDEBUG
    /* for checking the consistency of partial writes */
    CACHELINE_ALIGNED struct {
        size_t head;
        size_t n;
    } write_in_progress;
#endif 
} shm_spsc_ringbuf;


void shm_spsc_ringbuf_init(shm_spsc_ringbuf *b, size_t capacity);
void shm_spsc_ringbuf_consume(shm_spsc_ringbuf *b, size_t n);
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
