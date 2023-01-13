#include <assert.h>

#include "vamos-buffers/core/spsc_ringbuf.h"

extern int nondet(void);

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

    size_t elem_num = 0;
    while (1) {
        assert(shm_spsc_ringbuf_size(&r) == elem_num);
        if (nondet()) {
            /* CONSUME */
            assert(shm_spsc_ringbuf_free_num(&r) ==
                   shm_spsc_ringbuf_max_size(&r) - elem_num);
            assert(!shm_spsc_ringbuf_full(&r) ||
                   shm_spsc_ringbuf_free_num(&r) == 0);
            assert(shm_spsc_ringbuf_full(&r) ||
                   shm_spsc_ringbuf_free_num(&r) > 0);

            assert(shm_spsc_ringbuf_peek(&r, CAPACITY, &off, &len1, &len2) ==
                   elem_num);
            assert(shm_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) ==
                   elem_num);

            off = shm_spsc_ringbuf_read_off_nowrap(&r, &n);
            assert(elem_num == 0 || n > 0);
            assert(shm_spsc_ringbuf_size(&r) == 0 || n > 0);
            assert(n == 0 || shm_spsc_ringbuf_size(&r) > 0);

            if (n > 0) {
                assert(elem_num > 0);
                --elem_num;
                assert(shm_spsc_ringbuf_size(&r) > 0);
                assert(shm_spsc_ringbuf_size(&r) <=
                       shm_spsc_ringbuf_max_size(&r));
                shm_spsc_ringbuf_consume(&r, 1);
                assert(shm_spsc_ringbuf_size(&r) <
                       shm_spsc_ringbuf_max_size(&r));
            } else {
                assert(elem_num == 0);
                assert(shm_spsc_ringbuf_size(&r) == 0);
                assert(shm_spsc_ringbuf_free_num(&r) ==
                       shm_spsc_ringbuf_max_size(&r));
            }
        } else {
            /* write 1 elem */
            off = shm_spsc_ringbuf_write_off_nowrap(&r, &n);
            assert(shm_spsc_ringbuf_free_num(&r) <=
                   shm_spsc_ringbuf_max_size(&r));

            /* PUSH */
            if (n > 0) {
                assert(shm_spsc_ringbuf_size(&r) == elem_num);
                assert(!shm_spsc_ringbuf_full(&r));
                assert(elem_num <= shm_spsc_ringbuf_max_size(&r));
                assert(shm_spsc_ringbuf_peek(&r, 3, &off, &len1, &len2) ==
                       elem_num);
                assert(shm_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) ==
                       elem_num);

                ++elem_num;
                shm_spsc_ringbuf_write_finish(&r, 1);

                assert(elem_num <= shm_spsc_ringbuf_max_size(&r));
                assert(shm_spsc_ringbuf_peek(&r, 3, &off, &len1, &len2) ==
                       elem_num);
                assert(shm_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) ==
                       elem_num);
            } else {
                assert(elem_num == shm_spsc_ringbuf_max_size(&r));
                assert(shm_spsc_ringbuf_full(&r));
                assert(shm_spsc_ringbuf_size(&r) ==
                       shm_spsc_ringbuf_max_size(&r));
            }
            assert(shm_spsc_ringbuf_size(&r) > 0);
        }
    }
}
