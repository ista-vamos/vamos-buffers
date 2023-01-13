#undef NDEBUG
#include <assert.h>
#include <threads.h>

#include "vamos-buffers/core/spsc_ringbuf.h"

#define CAPACITY 3
int main(void) {
    shm_spsc_ringbuf r;
    size_t n, off, len1, len2;
    shm_spsc_ringbuf_init(&r, CAPACITY);
    assert(shm_spsc_ringbuf_capacity(&r) == CAPACITY);
    assert(shm_spsc_ringbuf_max_size(&r) == CAPACITY - 1);

    /* ringbuf is empty */
    assert(shm_spsc_ringbuf_size(&r) == 0);
    assert(shm_spsc_ringbuf_free_num(&r) == shm_spsc_ringbuf_max_size(&r));
    assert(!shm_spsc_ringbuf_full(&r));
    assert(shm_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) == 0);
    assert(shm_spsc_ringbuf_peek(&r, 3, &off, &len1, &len2) == 0);

    shm_spsc_ringbuf_read_off_nowrap(&r, &n);
    assert(n == 0);

    assert(shm_spsc_ringbuf_consume_upto(&r, CAPACITY) == 0);
    assert(shm_spsc_ringbuf_consume_upto(&r, 1) == 0);
    assert(shm_spsc_ringbuf_consume_upto(&r, 0) == 0);

    /* write 1 elem */
    off = shm_spsc_ringbuf_write_off_nowrap(&r, &n);
    assert(n == shm_spsc_ringbuf_max_size(&r));
    assert(off == 0);

    assert(shm_spsc_ringbuf_size(&r) == 0);
    assert(shm_spsc_ringbuf_free_num(&r) == shm_spsc_ringbuf_max_size(&r));
    assert(!shm_spsc_ringbuf_full(&r));
    assert(shm_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) == 0);
    assert(shm_spsc_ringbuf_peek(&r, 3, &off, &len1, &len2) == 0);

    shm_spsc_ringbuf_write_finish(&r, 1);

    /* ringbuf has 1 elem */
    assert(shm_spsc_ringbuf_size(&r) == 1);
    assert(shm_spsc_ringbuf_free_num(&r) == shm_spsc_ringbuf_max_size(&r) - 1);
    assert(!shm_spsc_ringbuf_full(&r));
    assert(shm_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) == 1);
    assert(shm_spsc_ringbuf_peek(&r, 3, &off, &len1, &len2) == 1);
    assert(len1 == 1);
    assert(len2 == 0);

    off = shm_spsc_ringbuf_read_off_nowrap(&r, &n);
    assert(n == 1);
    assert(off == 0);
    shm_spsc_ringbuf_consume_upto(&r, 1);

    /* ringbuf is empty */
    assert(shm_spsc_ringbuf_size(&r) == 0);
    assert(shm_spsc_ringbuf_free_num(&r) == shm_spsc_ringbuf_max_size(&r));
    assert(!shm_spsc_ringbuf_full(&r));
    assert(shm_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) == 0);
    assert(shm_spsc_ringbuf_peek(&r, 3, &off, &len1, &len2) == 0);

    shm_spsc_ringbuf_read_off_nowrap(&r, &n);
    assert(n == 0);

    assert(shm_spsc_ringbuf_consume_upto(&r, CAPACITY) == 0);
    assert(shm_spsc_ringbuf_consume_upto(&r, 1) == 0);
    assert(shm_spsc_ringbuf_consume_upto(&r, 0) == 0);

    /* write 1 elem */
    off = shm_spsc_ringbuf_write_off_nowrap(&r, &n);
    assert(n == shm_spsc_ringbuf_max_size(&r));

    assert(shm_spsc_ringbuf_size(&r) == 0);
    assert(shm_spsc_ringbuf_free_num(&r) == shm_spsc_ringbuf_max_size(&r));
    assert(!shm_spsc_ringbuf_full(&r));
    assert(shm_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) == 0);
    assert(shm_spsc_ringbuf_peek(&r, 3, &off, &len1, &len2) == 0);

    shm_spsc_ringbuf_write_finish(&r, 1);

    /* write 1 elem */
    off = shm_spsc_ringbuf_write_off_nowrap(&r, &n);
    assert(n == shm_spsc_ringbuf_max_size(&r) - 1);
    assert(off < CAPACITY);

    assert(shm_spsc_ringbuf_size(&r) == 1);
    assert(shm_spsc_ringbuf_free_num(&r) == shm_spsc_ringbuf_max_size(&r) - 1);
    assert(!shm_spsc_ringbuf_full(&r));
    assert(shm_spsc_ringbuf_peek(&r, CAPACITY, &off, &len1, &len2) == 1);
    assert(len1 == 1);
    assert(len2 == 0);
    assert(shm_spsc_ringbuf_peek(&r, CAPACITY + 2, &off, &len1, &len2) == 1);
    assert(len1 == 1);
    assert(len2 == 0);
    assert(shm_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) == 1);

    shm_spsc_ringbuf_write_finish(&r, 1);

    assert(shm_spsc_ringbuf_size(&r) == 2);
    assert(shm_spsc_ringbuf_free_num(&r) == shm_spsc_ringbuf_max_size(&r) - 2);
    assert(shm_spsc_ringbuf_full(&r));
    assert(shm_spsc_ringbuf_peek(&r, CAPACITY, &off, &len1, &len2) == 2);
    assert(len1 == 2);
    assert(len2 == 0);
    assert(shm_spsc_ringbuf_peek(&r, CAPACITY + 2, &off, &len1, &len2) == 2);
    assert(len1 == 2);
    assert(len2 == 0);
    assert(shm_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) == 2);

    /* write 1 elem -- should fail */
    off = shm_spsc_ringbuf_write_off_nowrap(&r, &n);
    assert(n == shm_spsc_ringbuf_max_size(&r) - 2);
    assert(n == 0);

    /* consume 1 */
    shm_spsc_ringbuf_consume(&r, 1);

    /* write 1 to wrap around */
    off = shm_spsc_ringbuf_write_off_nowrap(&r, &n);
    assert(n == shm_spsc_ringbuf_max_size(&r) - 1);
    assert(n == 1);
    assert(off < CAPACITY);

    assert(shm_spsc_ringbuf_size(&r) == 1);
    assert(shm_spsc_ringbuf_free_num(&r) == shm_spsc_ringbuf_max_size(&r) - 1);
    assert(!shm_spsc_ringbuf_full(&r));
    assert(shm_spsc_ringbuf_peek(&r, CAPACITY, &off, &len1, &len2) == 1);
    assert(len1 == 1);
    assert(len2 == 0);
    assert(shm_spsc_ringbuf_peek(&r, CAPACITY + 2, &off, &len1, &len2) == 1);
    assert(len1 == 1);
    assert(len2 == 0);
    assert(shm_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) == 1);

    shm_spsc_ringbuf_write_finish(&r, 1);

    assert(shm_spsc_ringbuf_size(&r) == 2);
    assert(shm_spsc_ringbuf_free_num(&r) == shm_spsc_ringbuf_max_size(&r) - 2);
    assert(shm_spsc_ringbuf_full(&r));
    assert(shm_spsc_ringbuf_peek(&r, CAPACITY, &off, &len1, &len2) == 2);
    assert(len1 == 1);
    assert(len2 == 1);
    assert(shm_spsc_ringbuf_peek(&r, CAPACITY + 2, &off, &len1, &len2) == 2);
    assert(len1 == 1);
    assert(len2 == 1);
    assert(shm_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) == 2);
}
