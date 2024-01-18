#include <assert.h>

#include "vamos-buffers/core/spsc_ringbuf.h"

extern int nondet(void);

#define CAPACITY 3
int main(void) {
    vms_spsc_ringbuf r;
    size_t n, off, len1, len2;
    vms_spsc_ringbuf_init(&r, CAPACITY);
    assert(vms_spsc_ringbuf_capacity(&r) == CAPACITY);
    assert(vms_spsc_ringbuf_max_size(&r) == CAPACITY - 1);

    /* ringbuf is empty */
    assert(vms_spsc_ringbuf_size(&r) == 0);
    assert(vms_spsc_ringbuf_free_num(&r) == vms_spsc_ringbuf_max_size(&r));
    assert(!vms_spsc_ringbuf_full(&r));
    assert(vms_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) == 0);
    assert(vms_spsc_ringbuf_peek(&r, 3, &off, &len1, &len2) == 0);

    vms_spsc_ringbuf_read_off_nowrap(&r, &n);
    assert(n == 0);

    assert(vms_spsc_ringbuf_consume_upto(&r, CAPACITY) == 0);
    assert(vms_spsc_ringbuf_consume_upto(&r, 1) == 0);
    assert(vms_spsc_ringbuf_consume_upto(&r, 0) == 0);

    size_t elem_num = 0;
    while (1) {
        assert(vms_spsc_ringbuf_size(&r) == elem_num);
        if (nondet()) {
            /* CONSUME */
            assert(vms_spsc_ringbuf_free_num(&r) ==
                   vms_spsc_ringbuf_max_size(&r) - elem_num);
            assert(!vms_spsc_ringbuf_full(&r) ||
                   vms_spsc_ringbuf_free_num(&r) == 0);
            assert(vms_spsc_ringbuf_full(&r) ||
                   vms_spsc_ringbuf_free_num(&r) > 0);

            assert(vms_spsc_ringbuf_peek(&r, CAPACITY, &off, &len1, &len2) ==
                   elem_num);
            assert(vms_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) ==
                   elem_num);

            off = vms_spsc_ringbuf_read_off_nowrap(&r, &n);
            assert(elem_num == 0 || n > 0);
            assert(vms_spsc_ringbuf_size(&r) == 0 || n > 0);
            assert(n == 0 || vms_spsc_ringbuf_size(&r) > 0);

            if (n > 0) {
                assert(elem_num > 0);
                --elem_num;
                assert(vms_spsc_ringbuf_size(&r) > 0);
                assert(vms_spsc_ringbuf_size(&r) <=
                       vms_spsc_ringbuf_max_size(&r));
                vms_spsc_ringbuf_consume(&r, 1);
                assert(vms_spsc_ringbuf_size(&r) <
                       vms_spsc_ringbuf_max_size(&r));
            } else {
                assert(elem_num == 0);
                assert(vms_spsc_ringbuf_size(&r) == 0);
                assert(vms_spsc_ringbuf_free_num(&r) ==
                       vms_spsc_ringbuf_max_size(&r));
            }
        } else {
            /* write 1 elem */
            off = vms_spsc_ringbuf_write_off_nowrap(&r, &n);
            assert(vms_spsc_ringbuf_free_num(&r) <=
                   vms_spsc_ringbuf_max_size(&r));

            /* PUSH */
            if (n > 0) {
                assert(vms_spsc_ringbuf_size(&r) == elem_num);
                assert(!vms_spsc_ringbuf_full(&r));
                assert(elem_num <= vms_spsc_ringbuf_max_size(&r));
                assert(vms_spsc_ringbuf_peek(&r, 3, &off, &len1, &len2) ==
                       elem_num);
                assert(vms_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) ==
                       elem_num);

                ++elem_num;
                vms_spsc_ringbuf_write_finish(&r, 1);

                assert(elem_num <= vms_spsc_ringbuf_max_size(&r));
                assert(vms_spsc_ringbuf_peek(&r, 3, &off, &len1, &len2) ==
                       elem_num);
                assert(vms_spsc_ringbuf_peek(&r, 0, &off, &len1, &len2) ==
                       elem_num);
            } else {
                assert(elem_num == vms_spsc_ringbuf_max_size(&r));
                assert(vms_spsc_ringbuf_full(&r));
                assert(vms_spsc_ringbuf_size(&r) ==
                       vms_spsc_ringbuf_max_size(&r));
            }
            assert(vms_spsc_ringbuf_size(&r) > 0);
        }
    }
}
