#include "vamos-buffers/core/spsc_ringbuf.h"

#include <assert.h>
#include <stdatomic.h>

#define __predict_false(x) __builtin_expect((x) != 0, 0)
#define __predict_true(x) __builtin_expect((x) != 0, 1)

void vms_spsc_ringbuf_init(vms_spsc_ringbuf *b, size_t capacity) {
    /*  we use one element as a separator */
    assert(capacity > 0);
    assert(capacity < SIZE_MAX - 1 && "Arith. in operations can overflow.");

    b->capacity = capacity;
    b->head = 0;
    b->tail = 0;
    b->seen_head = 0;
    b->seen_tail = 0;
}

static inline size_t _is_empty(size_t head, size_t tail) {
    return head == tail;
}

static inline size_t _is_full(size_t head, size_t tail, size_t capacity) {
    return head + 1 == tail || (head == capacity - 1 && tail == 0);
}

static inline size_t _get_written_num(size_t head, size_t tail,
                                      size_t capacity) {
    if (tail < head) {
        return head - tail;
    } else {
        if (__predict_false(tail == head)) {
            return 0;
        } else {
            return capacity - tail + head;
        }
    }
}

static inline size_t _get_free_num(size_t head, size_t tail, size_t capacity) {
    assert(head < capacity);
    assert(tail < capacity);

    if (tail < head) {
        return (capacity - head - 1) + tail;
    }

    if (__predict_false(head == tail)) {
        return capacity - 1;
    }

    return tail - head - 1;
}

#define CHECK_IF(c, t) assert(!(c) || (t))
#define CHECK_IFF(c, t) \
    do {                \
        CHECK_IF(c, t); \
        CHECK_IF(t, c); \
    } while (0)

static inline size_t get_written_num(size_t head, size_t tail,
                                     size_t capacity) {
    /* pre */
    assert(head < capacity);
    assert(tail < capacity);

    size_t ret = _get_written_num(head, tail, capacity);

    /* post */
    assert(ret < capacity);
    CHECK_IFF(_is_empty(head, tail), ret == 0);
    CHECK_IFF(_is_full(head, tail, capacity), ret == capacity - 1);
    CHECK_IF(tail < head, ret == head - tail);
    CHECK_IF(tail > head, ret == (head + (capacity - tail)));
    assert(ret == capacity - 1 - _get_free_num(head, tail, capacity));

    return ret;
}

static inline size_t get_free_num(size_t head, size_t tail, size_t capacity) {
    assert(head < capacity);
    assert(tail < capacity);

    size_t ret = _get_free_num(head, tail, capacity);

    assert(ret < capacity);
    CHECK_IFF(_is_empty(head, tail), ret == capacity - 1);
    CHECK_IFF(_is_full(head, tail, capacity), ret == 0);
    assert(ret == capacity - 1 - _get_written_num(head, tail, capacity));

    return ret;
}

size_t vms_spsc_ringbuf_capacity(vms_spsc_ringbuf *b) {
    /* we use one element as a separator */
    return b->capacity;
}

size_t vms_spsc_ringbuf_max_size(vms_spsc_ringbuf *b) {
    /* we use one element as a separator */
    return b->capacity - 1;
}

size_t vms_spsc_ringbuf_size(vms_spsc_ringbuf *b) {
    return get_written_num(atomic_load_explicit(&b->head, memory_order_relaxed),
                           atomic_load_explicit(&b->tail, memory_order_relaxed),
                           b->capacity);
}

bool vms_spsc_ringbuf_empty(vms_spsc_ringbuf *b) {
    return _is_empty(atomic_load_explicit(&b->head, memory_order_relaxed),
                     atomic_load_explicit(&b->tail, memory_order_relaxed));
}

size_t vms_spsc_ringbuf_free_num(vms_spsc_ringbuf *b) {
    return get_free_num(atomic_load_explicit(&b->head, memory_order_relaxed),
                        atomic_load_explicit(&b->tail, memory_order_relaxed),
                        b->capacity);
}

/**
 * @brief vms_spsc_ringbuf_full checks if the ringbuffer is full.
 * It ignores cached head/tail and queries the shared head/tail directly.
 * However, no synchronization is used (the memory order is relaxed).
 * @return true if the ringbuf is full else false
 */
bool vms_spsc_ringbuf_full(vms_spsc_ringbuf *b) {
    size_t head = atomic_load_explicit(&b->head, memory_order_relaxed);
    size_t tail = atomic_load_explicit(&b->tail, memory_order_relaxed);
    return __predict_false(_is_full(head, tail, b->capacity));
}

/* Compute the write offset (with wrapping). Returns the num of free elements */
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
        *n = capacity - 1;
        if (wrap_n) {
            *wrap_n = 0;
        }
        return capacity - 1;
    }

    *n = tail - head - 1;
    if (wrap_n) {
        *wrap_n = 0;
    }
    return tail - head - 1;
}

size_t vms_spsc_ringbuf_write_off(vms_spsc_ringbuf *b, size_t *n,
                                  size_t *wrap_n) {
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

size_t vms_spsc_ringbuf_write_off_nowrap(vms_spsc_ringbuf *b, size_t *n) {
    const size_t head = atomic_load_explicit(&b->head, memory_order_acquire);

    /* Update the cache if seen_tail is 0 (which very likely means it has not
     * been updated yet, but can be of course also after wrapping around) or if
     * there is no space left considering the cached information. */
    if (b->seen_tail == 0 ||
        get_write_off(head, b->seen_tail, b->capacity, n, NULL) == 0) {
        b->seen_tail = atomic_load_explicit(&b->tail, memory_order_relaxed);
        get_write_off(head, b->seen_tail, b->capacity, n, NULL);
    }

#ifndef NDEBUG
    b->write_in_progress.head = head;
    b->write_in_progress.n = *n;
#endif

    return head;
}

/* Ask for at least *n elements */
size_t vms_spsc_ringbuf_acquire(vms_spsc_ringbuf *b, size_t *n,
                                size_t *wrap_n) {
    const size_t head = atomic_load_explicit(&b->head, memory_order_acquire);

    size_t req = *n;
    if (get_write_off(head, b->seen_tail, b->capacity, n, wrap_n) < req) {
        b->seen_tail = atomic_load_explicit(&b->tail, memory_order_relaxed);
        get_write_off(head, b->seen_tail, b->capacity, n, wrap_n);
    }

#ifndef NDEBUG
    b->write_in_progress.head = head;
    b->write_in_progress.n = *n + *wrap_n;
#endif

    return head;
}

/* Ask for at least *n elements */
size_t vms_spsc_ringbuf_acquire_nowrap(vms_spsc_ringbuf *b, size_t *n) {
    const size_t head = atomic_load_explicit(&b->head, memory_order_acquire);

    size_t req = *n;
    if (get_write_off(head, b->seen_tail, b->capacity, n, NULL) < req) {
        b->seen_tail = atomic_load_explicit(&b->tail, memory_order_relaxed);
        get_write_off(head, b->seen_tail, b->capacity, n, NULL);
    }

#ifndef NDEBUG
    b->write_in_progress.head = head;
    b->write_in_progress.n = *n;
#endif

    return head;
}

void vms_spsc_ringbuf_write_finish(vms_spsc_ringbuf *b, size_t n) {
    assert(n <= b->capacity);
    assert((b->capacity < (~((size_t)0)) - n) && "Possible overflow");
#ifndef NDEBUG
    assert(b->write_in_progress.head == b->head &&
           "Something moved after write_off was called");
    assert(b->write_in_progress.n >= n &&
           "Trying to write more items than returned by write_off()");
#endif

    const size_t c = b->capacity;
    size_t new_head = atomic_load_explicit(&b->head, memory_order_relaxed) + n;

    /* buffer full, rotate it */
    if (__predict_false(new_head >= c)) {
        new_head -= c;
    }

    assert(new_head != atomic_load_explicit(&b->tail, memory_order_relaxed) &&
           "Invalid head move");

    atomic_store_explicit(&b->head, new_head, memory_order_release);
}

size_t vms_spsc_ringbuf_read_off_nowrap(vms_spsc_ringbuf *b, size_t *n) {
    const size_t tail = atomic_load_explicit(&b->tail, memory_order_acquire);
    size_t tmp = get_written_num(b->seen_head, tail, b->capacity);
    if (tmp == 0) {
        b->seen_head = atomic_load_explicit(&b->head, memory_order_acquire);
        tmp = get_written_num(b->seen_head, tail, b->capacity);
    }

    *n = tmp;

    return tail;
}

size_t vms_spsc_ringbuf_read_acquire(vms_spsc_ringbuf *b, size_t *n) {
    const size_t tail = atomic_load_explicit(&b->tail, memory_order_acquire);
    size_t tmp = get_written_num(b->seen_head, tail, b->capacity);
    if (tmp < *n) {
        b->seen_head = atomic_load_explicit(&b->head, memory_order_acquire);
        tmp = get_written_num(b->seen_head, tail, b->capacity);
    }

    *n = tmp;

    return tail;
}

/*
 * Consume n items from the ringbuffer. There must be at least n items.
 */
void vms_spsc_ringbuf_consume(vms_spsc_ringbuf *b, size_t n) {
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
 * Consume up to n items from the ringbuffer. Return the number of consumed
 * events.
 */
size_t vms_spsc_ringbuf_consume_upto(vms_spsc_ringbuf *b, size_t n) {
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
        vms_spsc_ringbuf_consume(b, n);
    }

    return n;
}

/* If return value is 0, values *off, *len1 and *len2 may not have been set */
size_t vms_spsc_ringbuf_peek(vms_spsc_ringbuf *b, size_t n, size_t *off,
                             size_t *len1, size_t *len2) {
    const size_t tail = atomic_load_explicit(&b->tail, memory_order_acquire);
    size_t head = b->seen_head;
    size_t cur_elem_num = get_written_num(head, tail, b->capacity);
    /* update the information if needed or when n == 0 (which means we want to
     * get an up-to-date the number of elements) */
    if (cur_elem_num < n || n == 0) {
        b->seen_head = head =
            atomic_load_explicit(&b->head, memory_order_acquire);
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
