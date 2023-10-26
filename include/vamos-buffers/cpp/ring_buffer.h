#ifndef VAMOS_CPP_RING_BUFFER_H
#define VAMOS_CPP_RING_BUFFER_H

#include <atomic>
#include <cassert>

namespace vamos {

#define __predict_false(x) __builtin_expect((x) != 0, 0)
#define __predict_true(x) __builtin_expect((x) != 0, 1)

/* On x86 and ARM the cache line has 64 bytes, change if needed. */
#define CACHELINE_SIZE 64
#define CACHELINE_ALIGNED alignas(CACHELINE_SIZE)
#define ADDR_IS_CACHE_ALIGNED(addr) \
    (((unsigned long long)(addr)) % CACHELINE_SIZE == 0)

/**
 * Single-producer single-consumer (SPSC) lock-free concurrent ring-buffer
 * of a fixed size;
 * This implementation is rewritten `spsc_ringbuf.h`, but it is not kept
 * in sync, so the two implementations might diverge.
 */

namespace {
static inline size_t _is_empty(size_t head, size_t tail) {
    return head == tail;
}

template <size_t capacity>
size_t _is_full(size_t head, size_t tail) {
    return head + 1 == tail || (head == capacity - 1 && tail == 0);
}

template <size_t capacity>
size_t __written_num(size_t head, size_t tail) {
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

template <size_t capacity>
size_t __free_num(size_t head, size_t tail) {
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

template <size_t capacity>
size_t _written_num(size_t head, size_t tail) {
    /* pre */
    assert(head < capacity);
    assert(tail < capacity);

    size_t ret = __written_num<capacity>(head, tail);

    /* post */
    assert(ret < capacity);
    CHECK_IFF(_is_empty(head, tail), ret == 0);
    CHECK_IFF(_is_full<capacity>(head, tail), ret == capacity - 1);
    CHECK_IF(tail < head, ret == head - tail);
    CHECK_IF(tail > head, ret == (head + (capacity - tail)));
    assert(ret == capacity - 1 - __free_num<capacity>(head, tail));

    return ret;
}

template <size_t capacity>
size_t _free_num(size_t head, size_t tail) {
    assert(head < capacity);
    assert(tail < capacity);

    size_t ret = __free_num<capacity>(head, tail);

    assert(ret < capacity);
    CHECK_IFF(_is_empty(head, tail), ret == capacity - 1);
    CHECK_IFF(_is_full<capacity>(head, tail), ret == 0);
    assert(ret == capacity - 1 - __written_num<capacity>(head, tail));

    return ret;
}

}  // namespace

template <uint64_t CAPACITY, uint64_t ELEM_SIZE>
class RingBuffer {
    CACHELINE_ALIGNED std::atomic<size_t> _tail;
    CACHELINE_ALIGNED std::atomic<size_t> _head;

   public:
    static constexpr uint64_t Capacity = CAPACITY;

    constexpr uint64_t capacity() const { return CAPACITY; }

    RingBuffer() : _tail(0), _head(0) { static_assert(CAPACITY > 0); }

    size_t load_head(std::memory_order mo) const { return _head.load(mo); }

    size_t load_tail(std::memory_order mo) const { return _tail.load(mo); }

    void set_head(size_t new_head, std::memory_order mo) {
        _head.store(new_head, mo);
    }

    void set_tail(size_t new_tail, std::memory_order mo) {
        _tail.store(new_tail, mo);
    }

    bool is_full() const {
        return __predict_false(
            _is_full<CAPACITY>(load_head(std::memory_order_relaxed),
                               load_tail(std::memory_order_relaxed)));
    }
};

template <typename RingBufferTy>
class RingBufferAccessor {
    static constexpr uint64_t _capacity = RingBufferTy::Capacity;

   protected:
    RingBufferTy &rb;
    // the cache for the head/tail last seen by this thread
    size_t seen{0};

   public:
    RingBufferAccessor(RingBufferTy &rb)
        : rb(rb) {}  //, _capacity(rb.capacity()) {}

    constexpr size_t capacity() const { return _capacity; }

    size_t max_size() const {
        /* we use one element as a separator */
        return capacity() - 1;
    }

    size_t size() const {
        return _written_num<RingBufferTy::Capacity>(
            rb.load_head(std::memory_order_relaxed),
            rb.load_tail(std::memory_order_relaxed));
    }

    bool empty() const {
        return _is_empty<RingBufferTy::Capacity>(
            rb.load_head(std::memory_order_relaxed),
            rb.load_tail(std::memory_order_relaxed));
    }

    size_t free_num() const {
        return _free_num<RingBufferTy::Capacity>(
            rb.load_head(std::memory_order_relaxed),
            rb.load_tail(std::memory_order_relaxed));
    }
};

namespace {
/* Compute the write offset (with wrapping). Returns the num of free elements */
template <uint64_t capacity>
size_t _write_off(size_t head, size_t tail, size_t &n, size_t *wrap_n) {
    if (tail < head) {
        if (__predict_false(tail == 0)) {
            n = capacity - head - 1;
            if (wrap_n) {
                *wrap_n = 0;
            }
            return capacity - head - 1;
        }

        n = capacity - head;
        if (wrap_n) {
            *wrap_n = tail - 1;
        }
        return capacity - head + tail - 1;
    }

    if (__predict_false(tail == head)) {
        n = capacity - 1;
        if (wrap_n) {
            *wrap_n = 0;
        }
        return capacity - 1;
    }

    n = tail - head - 1;
    if (wrap_n) {
        *wrap_n = 0;
    }
    return tail - head - 1;
}

}  // namespace

template <typename RingBufferTy>
class RingBufferWriter : public RingBufferAccessor<RingBufferTy> {
   public:
    RingBufferWriter(RingBufferTy &rb) : RingBufferAccessor<RingBufferTy>(rb) {}

    /* Return the offset for writing and set the number of writable elements to
     * `n` and also to `wrap_n` if there is a wrap-around the end of the buffer
     */
    size_t write_off(size_t &n, size_t *wrap_n) {
        const size_t head = this->rb.load_head(std::memory_order_acquire);

        if (_write_off<RingBufferTy::Capacity>(head, this->seen, n, wrap_n) ==
            0) {
            this->seen = this->rb.load_tail(std::memory_order_relaxed);
            _write_off<RingBufferTy::Capacity>(head, this->seen, n, wrap_n);
        }

        return head;
    }

    size_t write_off(size_t &n) {
        const size_t head = this->rb.load_head(std::memory_order_acquire);

        /* Update the cache if seen_tail is 0 (which very likely means it has
         * not been updated yet, but can be of course also after wrapping
         * around) or if there is no space left considering the cached
         * information. */
        if (this->seen == 0 || _write_off<RingBufferTy::Capacity>(
                                   head, this->seen, n, NULL) == 0) {
            this->seen = this->rb.load_tail(std::memory_order_relaxed);
            _write_off<RingBufferTy::Capacity>(head, this->seen, n, NULL);
        }

        return head;
    }

    /* Ask for at least *n elements */
    size_t acquire(size_t &n, size_t *wrap_n) {
        const size_t head = this->rb.load_head(std::memory_order_acquire);

        size_t req = n;
        if (_write_off<RingBufferTy::Capacity>(head, this->seen, n, wrap_n) <
            req) {
            this->seen = this->rb.load_tail(std::memory_order_relaxed);
            _write_off<RingBufferTy::Capacity>(head, this->seen, n, wrap_n);
        }

        return head;
    }

    /* Ask for at least *n elements */
    size_t acquire(size_t &n) {
        const size_t head = this->rb.load_head(std::memory_order_acquire);

        size_t req = n;
        if (_write_off<RingBufferTy::Capacity>(head, this->seen, n, NULL) <
            req) {
            this->seen = this->rb.load_tail(std::memory_order_relaxed);
            _write_off<RingBufferTy::Capacity>(head, this->seen, n, NULL);
        }
        return head;
    }

    void write_finish(size_t n) {
        constexpr auto c = RingBufferTy::Capacity;
        assert(n <= c);
        assert((c < (~((size_t)0)) - n) && "Possible overflow");

        size_t new_head = this->rb.load_head(std::memory_order_relaxed) + n;

        /* buffer full, rotate it */
        if (__predict_false(new_head >= c)) {
            new_head -= c;
        }

        assert(new_head != this->rb.load_tail(std::memory_order_relaxed) &&
               "Invalid head move");

        this->rb.set_head(new_head, std::memory_order_release);
    }
};

template <typename RingBufferTy>
class RingBufferReader : public RingBufferAccessor<RingBufferTy> {
   public:
    RingBufferReader(RingBufferTy &rb) : RingBufferAccessor<RingBufferTy>(rb) {}

    size_t read_off(size_t &n) {
        const size_t tail = this->rb.load_tail(std::memory_order_acquire);
        size_t tmp = _written_num<RingBufferTy::Capacity>(this->seen, tail);
        if (tmp == 0) {
            this->seen = this->rb.load_head(std::memory_order_acquire);
            tmp = _written_num<RingBufferTy::Capacity>(this->seen, tail);
        }

        n = tmp;

        return tail;
    }

    size_t available() const {
        return _written_num<RingBufferTy::Capacity>(
            this->rb.load_head(std::memory_order_relaxed),
            this->rb.load_tail(std::memory_order_relaxed));
    }

    size_t acquire(size_t &n) {
        const size_t tail = this->rb.load_tail(std::memory_order_acquire);
        size_t tmp = _written_num<RingBufferTy::Capacity>(this->seen, tail);
        if (tmp < n) {
            this->seen = this->rb.load_head(std::memory_order_acquire);
            tmp = _written_num<RingBufferTy::Capacity>(this->seen, tail);
        }

        n = tmp;

        return tail;
    }

    /*
     * Consume n items from the ringbuffer. There must be at least n items.
     */
    void consume(size_t n) {
        assert(n > 0 && "Consume 0 elems");
        assert(n <= this->capacity());
        assert((this->capacity() < (~((size_t)0)) - n) && "Possible overflow");

        size_t new_tail = this->rb.load_tail(std::memory_order_relaxed) + n;
        if (__predict_false(new_tail >= this->capacity())) {
            new_tail -= this->capacity();
        }

        this->rb.set_tail(new_tail, std::memory_order_release);
    }

    /* If return value is 0, values *off, *len1 and *len2 may not have been set
     */
    size_t peek(size_t n, size_t &off, size_t &len1, size_t &len2) {
        const size_t tail = this->rb.load_tail(std::memory_order_acquire);
        size_t head = this->seen;
        size_t cur_elem_num = _written_num<RingBufferTy::Capacity>(head, tail);
        /* update the information if needed or when n == 0 (which means we want
         * to get an up-to-date the number of elements) */
        if (cur_elem_num < n || n == 0) {
            this->seen = head = this->rb.load_head(std::memory_order_acquire);
            cur_elem_num = _written_num<RingBufferTy::Capacity>(head, tail);
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
            len1 = n;
            len2 = 0;
        } else {
            assert(tail != head && "Ringbuf is empty");
            constexpr size_t c = this->capacity();
            const size_t l1 = c - tail > n ? n : c - tail;
            len1 = l1;
            len2 = head < (n - l1) ? head : n - l1;
        }

        assert(len1 + len2 == n);
        off = tail;

        return cur_elem_num;
    }
};

}  // namespace vamos

#endif
