#include <stdatomic.h>
#include <assert.h>
#include "spsc_ringbuf.h"

#define __predict_false(x) __builtin_expect((x) != 0, 0)
#define __predict_true(x)  __builtin_expect((x) != 0, 1)

void shm_spsc_ringbuf_init(shm_spsc_ringbuf *b, size_t capacity) {
    /* NOTE: we use one element as a separator */
    b->capacity = capacity;
    b->head = 0;
    b->tail = 0;
}

size_t shm_spsc_ringbuf_capacity(shm_spsc_ringbuf *b) {
    return b->capacity;
}

size_t shm_spsc_ringbuf_size(shm_spsc_ringbuf *b) {
    size_t head = atomic_load_explicit(&b->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&b->tail, memory_order_relaxed);

    if (tail < head)
        return head - tail;

    return head == tail ? 0 : b->capacity - tail + head;
}

size_t shm_spsc_ringbuf_free_num(shm_spsc_ringbuf *b) {
    const size_t c = b->capacity;
    size_t head = atomic_load_explicit(&b->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&b->tail, memory_order_relaxed);

    if (tail < head)
        return c - head + tail - 1;

    return head == tail ? c : tail - head - 1;
}

bool shm_spsc_ringbuf_full(shm_spsc_ringbuf *b) {
    size_t head = atomic_load_explicit(&b->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&b->tail, memory_order_relaxed);
    return __predict_false(tail + 1 == head);
}

size_t shm_spsc_ringbuf_write_off(shm_spsc_ringbuf *b, size_t *n, size_t *wrap_n) {
    const size_t head = atomic_load_explicit(&b->head, memory_order_relaxed);
    /* TODO: isn't relaxed OK here if we do memory barrier in write_finish? */
    const size_t tail = atomic_load_explicit(&b->tail, memory_order_acquire);

    if (tail < head) {
        if (__predict_false(tail == 0)) {
            *n = b->capacity - head - 1;
            *wrap_n = 0;
        } else {
            *n = b->capacity - head;
            *wrap_n = tail - 1;
        }
    } else {
        if (__predict_false(tail == head)) {
            *n = b->capacity;
        } else {
            *n = tail - head - 1;
            *wrap_n = 0;
        }
    }
#ifndef NDEBUG
     b->write_in_progress.head = head;
     b->write_in_progress.n = *n + *wrap_n;
#endif

    return head;
}

size_t shm_spsc_ringbuf_write_off_nowrap(shm_spsc_ringbuf *b, size_t *n) {
    const size_t head = atomic_load_explicit(&b->head, memory_order_relaxed);
    /* TODO: isn't relaxed OK here if we do memory barrier in write_finish? */
    const size_t tail = atomic_load_explicit(&b->tail, memory_order_acquire);

    if (tail < head) {
        if (__predict_false(tail == 0)) {
            *n = b->capacity - head - 1;
        } else {
            *n = b->capacity - head;
        }
    } else {
        if (__predict_false(tail == head)) {
            *n = b->capacity;
        } else {
            *n = tail - head - 1;
        }
    }
#ifndef NDEBUG
     b->write_in_progress.head = head;
     b->write_in_progress.n = *n;
#endif

    return head;
}

void shm_spsc_ringbuf_write_finish(shm_spsc_ringbuf *b, size_t n) {
    assert(n <= b->capacity);
    assert((b->capacity < (~((size_t)0)) - n) && "Possible overflow");
#ifndef NDEBUG
    assert(b->write_in_progress.head == b->head
           && "Something moved after write_off was called");
    assert(b->write_in_progress.n >= n &&
           "Trying to write more items than returned by write_off()");
#endif

    const size_t c = b->capacity;
    size_t new_head = atomic_load_explicit(&b->head, memory_order_relaxed) + n;

    /* buffer full, rotate it */
    if (__predict_false(new_head >= c)) {
        new_head -= c;
    }

    assert(new_head != atomic_load_explicit(&b->tail, memory_order_relaxed)
            && "Invalid head move");

    atomic_store_explicit(&b->head, new_head, memory_order_release);
}

size_t shm_spsc_ringbuf_read_off_nowrap(shm_spsc_ringbuf *b, size_t *n) {
    const size_t tail = atomic_load_explicit(&b->tail, memory_order_relaxed);
    const size_t head = atomic_load_explicit(&b->head, memory_order_acquire);

    if (tail < head) {
        *n = head - tail;
    } else {
        if (__predict_false(tail == head)) {
            *n = 0;
        } else {
            *n = b->capacity - tail + head;
        }
    }

    return tail;
}


/*
 * Consume n items from the ringbuffer. There must be at least n items.
 */
void shm_spsc_ringbuf_consume(shm_spsc_ringbuf *b, size_t n) {
    assert(n <= b->capacity);
    assert((b->capacity < (~((size_t)0)) - n) && "Possible overflow");

    const size_t c = b->capacity;
    size_t new_tail = atomic_load_explicit(&b->tail, memory_order_relaxed) + n;
    if (__predict_false(new_tail >= c)) {
        new_tail -= c;
    }

    atomic_store_explicit(&b->tail, new_tail, memory_order_release);
}

size_t shm_spsc_ringbuf_peek(shm_spsc_ringbuf *b,
                             size_t n, size_t *off,
                             size_t *len1, size_t *len2) {
    const size_t tail = atomic_load_explicit(&b->tail, memory_order_relaxed);
    const size_t head = atomic_load_explicit(&b->head, memory_order_relaxed);

    if (tail < head) {
        *len1 = head - tail > n ? n : head - tail;
        *len2 = 0;
    } else {
        if (__predict_false(tail == head)) {
            *len1 = 0;
            *len2 = 0;
        } else {
            const size_t c = b->capacity;
            *len1 = c - tail > n ? n : c - tail;
            *len2 = head < (n - *len1) ? head : n - *len1;
        }
    }

    assert(*len1 + *len2 <= n);

    *off = tail;

    /* return the number of elements in the rb */
    if (tail < head)
        return head - tail;

    return head == tail ? 0 : b->capacity - tail + head;
}

