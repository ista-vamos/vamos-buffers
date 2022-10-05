#include <stdatomic.h>
#include <assert.h>
#include "spsc_ringbuf.h"

#define __predict_false(x) __builtin_expect((x) != 0, 0)
#define __predict_true(x)  __builtin_expect((x) != 0, 1)

void shm_spsc_ringbuf_init(shm_spsc_ringbuf *b, size_t capacity) {
    /*  we use one element as a separator */
    assert(capacity > 1);

    b->capacity = capacity;
    b->head = 0;
    b->tail = 0;
    b->seen_head = 0;
    b->seen_tail = 0;
}

static inline size_t get_written_num(size_t head, size_t tail, size_t capacity) {
    /* TODO check on init that capacity is such that this expr cannot underflow */
    ssize_t num = ((ssize_t)head - (ssize_t)tail);
    if (num < 0)
        return num + capacity;
    return num;
    /*
    if (tail < head) {
        return head - tail;
    } else {
        if (__predict_false(tail == head)) {
             return 0;
        } else {
            return capacity - tail + head;
        }
    }
    */
}

static inline size_t get_free_num(size_t head, size_t tail, size_t capacity) {
    if (tail < head)
        return capacity - head + tail - 1;

    if (__predict_false(head == tail)) {
        return capacity;
    }

    return tail - head - 1;
}

size_t shm_spsc_ringbuf_capacity(shm_spsc_ringbuf *b) {
    /* we use one element as a separator */
    return b->capacity - 1;
}

size_t shm_spsc_ringbuf_size(shm_spsc_ringbuf *b) {
    return get_written_num(atomic_load_explicit(&b->head, memory_order_relaxed),
                           atomic_load_explicit(&b->tail, memory_order_relaxed),
                           b->capacity);
}

size_t shm_spsc_ringbuf_free_num(shm_spsc_ringbuf *b) {
    return get_free_num(atomic_load_explicit(&b->head, memory_order_relaxed),
                        atomic_load_explicit(&b->tail, memory_order_relaxed),
                        b->capacity);
}

bool shm_spsc_ringbuf_full(shm_spsc_ringbuf *b) {
    size_t head = atomic_load_explicit(&b->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&b->tail, memory_order_relaxed);
    return __predict_false(head == tail + 1 || (head == b->capacity - 1 && tail == 0));
}

static inline size_t get_write_off(size_t head, size_t tail, size_t capacity,
                                   size_t *n, size_t *wrap_n) {
    if (tail < head) {
        if (__predict_false(tail == 0)) {
            *n = capacity - head - 1;
            if (wrap_n) {
                *wrap_n = 0;
            }
            return capacity - head - 1;
        }

        *n = capacity - head;
        if (wrap_n) {
             *wrap_n = tail - 1;
        }
        return capacity - head + tail - 1;
    }

    if (__predict_false(tail == head)) {
        *n = capacity;
        if (wrap_n) {
            *wrap_n = 0;
        }
        return capacity;
    }

    *n = tail - head - 1;
    if (wrap_n) {
        *wrap_n = 0;
    }
    return tail - head - 1;
}

size_t shm_spsc_ringbuf_write_off(shm_spsc_ringbuf *b, size_t *n, size_t *wrap_n) {
    const size_t head = atomic_load_explicit(&b->head, memory_order_acquire);

    if (get_write_off(head, b->seen_tail, b->capacity, n, wrap_n) == 0) {
        b->seen_tail = atomic_load_explicit(&b->tail, memory_order_relaxed);
        get_write_off(head, b->seen_tail, b->capacity, n, wrap_n);
    }

#ifndef NDEBUG
     b->write_in_progress.head = head;
     b->write_in_progress.n = *n + *wrap_n;
#endif

    return head;
}

size_t shm_spsc_ringbuf_write_off_nowrap(shm_spsc_ringbuf *b, size_t *n) {
    const size_t head = atomic_load_explicit(&b->head, memory_order_acquire);

    if (get_write_off(head, b->seen_tail, b->capacity, n, NULL) == 0) {
        b->seen_tail = atomic_load_explicit(&b->tail, memory_order_relaxed);
        get_write_off(head, b->seen_tail, b->capacity, n, NULL);
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
    const size_t tail = atomic_load_explicit(&b->tail, memory_order_acquire);
    size_t tmp = get_written_num(b->seen_head, tail, b->capacity);
    if (tmp == 0) {
        b->seen_head = atomic_load_explicit(&b->head, memory_order_acquire);
        tmp = get_written_num(b->seen_head, tail, b->capacity);
    }

    *n = tmp;

    return tail;
}


/*
 * Consume n items from the ringbuffer. There must be at least n items.
 */
void shm_spsc_ringbuf_consume(shm_spsc_ringbuf *b, size_t n) {
    assert(n > 0 && "Consume 0 elems");
    assert(n <= b->capacity);
    assert((b->capacity < (~((size_t)0)) - n) && "Possible overflow");

    const size_t c = b->capacity;
    size_t new_tail = atomic_load_explicit(&b->tail, memory_order_relaxed) + n;
    if (__predict_false(new_tail >= c)) {
        new_tail -= c;
    }

    atomic_store_explicit(&b->tail, new_tail, memory_order_release);
}


/*
 * Consume up to n items from the ringbuffer. Return the number of consumed events.
 */
size_t shm_spsc_ringbuf_consume_upto(shm_spsc_ringbuf *b, size_t n) {
    assert(n <= b->capacity);
    assert((b->capacity < (~((size_t)0)) - n) && "Possible overflow");

    const size_t tail = atomic_load_explicit(&b->tail, memory_order_acquire);
    size_t k = get_written_num(b->seen_head, tail, b->capacity);
    if (k < n) {
        b->seen_head = atomic_load_explicit(&b->head, memory_order_acquire);
        k = get_written_num(b->seen_head, tail, b->capacity);
    }

    if (k < n) {
        n = k;
    }

    if (n > 0) {
        shm_spsc_ringbuf_consume(b, n);
    }

    return n;
}

size_t shm_spsc_ringbuf_peek(shm_spsc_ringbuf *b,
                             size_t n, size_t *off,
                             size_t *len1, size_t *len2) {
    const size_t tail = atomic_load_explicit(&b->tail, memory_order_acquire);
    size_t head = b->seen_head;
    size_t cur_elem_num = get_written_num(head, tail, b->capacity);
    if (cur_elem_num < n) {
        b->seen_head = head = atomic_load_explicit(&b->head, memory_order_acquire);
        cur_elem_num = get_written_num(head, tail, b->capacity);
    }

    if (n == 0)
        return cur_elem_num;

    if (__predict_false(cur_elem_num == 0))
        return 0;

    if (cur_elem_num < n) {
        n = cur_elem_num;
    }

    assert(n <= cur_elem_num);

    if (tail < head) {
        *len1 = n;
        *len2 = 0;
    } else {
        assert(tail != head && "Ringbuf is empty");
        const size_t c = b->capacity;
        const size_t l1 = c - tail > n ? n : c - tail;
        *len1 = l1;
        *len2 = head < (n - l1) ? head : n - l1;
    }

    assert(*len1 + *len2 == n);
    *off = tail;

    return cur_elem_num;
}
