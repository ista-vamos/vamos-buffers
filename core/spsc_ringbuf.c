#include <stdatomic.h>
#include <assert.h>
#include "spsc_ringbuf.h"

#define __predict_false(x) __builtin_expect((x) != 0, 0)
#define __predict_true(x)  __builtin_expect((x) != 0, 1)

static inline void elem_num_inc(shm_spsc_ringbuf *b, int k) {
    /* Do ++b->elem_num atomically. */
    /* The increment must come after everything is done.
       The release order makes sure that the written element
       is visible to other threads by now. */
    atomic_fetch_add_explicit(&b->elem_num, k, memory_order_release);
}

static inline void elem_num_dec(shm_spsc_ringbuf *b, int k) {
    /* Do b->elem_num -= k atomically. */
    atomic_fetch_sub_explicit(&b->elem_num, k, memory_order_acquire);
}

static inline size_t elem_num(shm_spsc_ringbuf *b) {
    return atomic_load_explicit(&b->elem_num, memory_order_relaxed);
}


void shm_spsc_ringbuf_init(shm_spsc_ringbuf *b, size_t capacity) {
    b->elem_num = 0;
    b->capacity = capacity;
    b->head = b->tail = 0;

}

size_t shm_spsc_ringbuf_capacity(shm_spsc_ringbuf *b) {
    return b->capacity;
}

size_t shm_spsc_ringbuf_size(shm_spsc_ringbuf *b) {
    return elem_num(b);
}

size_t shm_spsc_ringbuf_free_num(shm_spsc_ringbuf *b) {
    return b->capacity - elem_num(b);
}

bool shm_spsc_ringbuf_full(shm_spsc_ringbuf *b) {
    return __predict_false(b->capacity == elem_num(b));
}

size_t shm_spsc_ringbuf_write_off(shm_spsc_ringbuf *b, size_t *n, size_t *wrap_n) {
    const size_t c = b->capacity;
    const size_t h = b->head;
    const size_t len = c - h;
    const size_t f = c - elem_num(b);

    *n = len;

    if (f > len) {
        *wrap_n = f - len;
    } else {
        *wrap_n = 0;
    }

#ifndef NDEBUG
    b->partial_head = h;
    b->partial_write_n = *n + *wrap_n;
#endif

    return h;
}

size_t shm_spsc_ringbuf_write_off_nowrap(shm_spsc_ringbuf *b, size_t *n) {
    const size_t c = b->capacity;
    const size_t h = b->head;
    const size_t len = c - h;
    const size_t f = c - elem_num(b);

    *n = f < len ? f : len;

#ifndef NDEBUG
    b->partial_head = h;
    b->partial_write_n = *n;
#endif

    assert(*n <= f);
    return h;
}


void shm_spsc_ringbuf_write_finish(shm_spsc_ringbuf *b, size_t n) {
    assert(n <= b->capacity);
    assert(n <= shm_spsc_ringbuf_free_num(b));
#ifndef NDEBUG
    assert(b->partial_head == b->head
           && "Something moved after write_off was called");
    assert(b->partial_write_n >= n &&
           "Trying to write more items than returned by write_off()");
#endif

    const size_t c = b->capacity;
    b->head += n;

    /* buffer full, rotate it */
    if (__predict_false(b->head >= c)) {
        b->head -= c;
    }

    elem_num_inc(b, n);
}

size_t shm_spsc_ringbuf_read_off_nowrap(shm_spsc_ringbuf *b, size_t *n) {
    const size_t c = b->capacity;
    const size_t t = b->tail;
    const size_t len = c - t;
    const size_t num = elem_num(b);

    *n = num < len ? num : len;

    assert(*n <= num);
    return t;
}

size_t shm_spsc_ringbuf_peek(shm_spsc_ringbuf *b, size_t n,
                             size_t *off, size_t *len1, size_t *len2) {

    const size_t cur_elem_num = elem_num(b);
    if (n == 0 || n > cur_elem_num) {
        n = cur_elem_num;
    }
    const size_t end = n + b->tail;
    const size_t c = b->capacity;
    *off = b->tail;

    if (__predict_false(end >= c)) {
        *len1 = c - b->tail;
        *len2 = end - c;
    } else {
        *len1 = n;
        *len2 = 0;
    }
    assert(__predict_true(*len1 + *len2 == n));

    return cur_elem_num;
}



/* Consume n (or less if there is not n items) from the ringbuffer.
 * Return the number of consumed items
 */
size_t shm_spsc_ringbuf_consume(shm_spsc_ringbuf *b, size_t n) {
    size_t num = elem_num(b);
    assert(n > 0);
    assert(n <= num);

    if (num < n) {
        n = num;
    }

    const size_t c = b->capacity;
    b->tail += n;
    if (__predict_false(b->tail >= c)) {
        b->tail -= c;
    }

    assert(elem_num(b) >= n);
    elem_num_dec(b, n);

    return n;
}
