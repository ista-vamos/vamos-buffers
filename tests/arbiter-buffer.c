#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "vamos-buffers/core/arbiter.h"
#include "vamos-buffers/core/stream.h"
#include "vamos-buffers/shmbuf/buffer.h"

static bool is_ready(vms_stream *s) {
    (void)s;
    return false;
}

struct event {
    vms_event base;
    int i;
};

struct buffer *initialize_local_buffer(const char *key, size_t elem_size,
                                       size_t capacity,
                                       struct source_control *control);

int main(void) {
    struct buffer *lbuffer =
        initialize_local_buffer("/dummy", sizeof(struct event), 30, NULL);
    assert(lbuffer);

    vms_stream dummy_stream;
    vms_stream *stream = &dummy_stream;

    vms_stream_init(stream, lbuffer, sizeof(struct event), is_ready, NULL, NULL,
                    NULL, NULL, "dummy-stream", "dummy");

    vms_arbiter_buffer *b =
        vms_arbiter_buffer_create(stream, sizeof(struct event), 20);
    vms_arbiter_buffer_set_active(b, true);

    struct event ev;
    for (int i = 1; i < 21; ++i) {
        ev.base.id = i;
        ev.i = i;
        vms_arbiter_buffer_push(b, &ev, sizeof(struct event));
    }

    assert(vms_arbiter_buffer_size(b) == 20);
    assert(vms_arbiter_buffer_drop_older_than(b, 10) == 10);
    assert(vms_arbiter_buffer_size(b) == 10);
    assert(vms_arbiter_buffer_drop(b, 20) == 10);
    assert(vms_arbiter_buffer_size(b) == 0);

    for (int i = 1; i < 21; ++i) {
        ev.base.id = 20 + 2 * i;
        ev.i = i;
        vms_arbiter_buffer_push(b, &ev, sizeof(struct event));
    }

    assert(vms_arbiter_buffer_size(b) == 20);
    assert(vms_arbiter_buffer_drop_older_than(b, 39) == 9);
    assert(vms_arbiter_buffer_size(b) == 11);
    assert(vms_arbiter_buffer_drop_older_than(b, 58) == 10);
    assert(vms_arbiter_buffer_size(b) == 1);
    assert(vms_arbiter_buffer_drop_older_than(b, 70) == 1);
    assert(vms_arbiter_buffer_size(b) == 0);

    for (int i = 1; i < 21; ++i) {
        ev.base.id = 40 + 2 * i;
        ev.i = i;
        vms_arbiter_buffer_push(b, &ev, sizeof(struct event));
    }

    assert(vms_arbiter_buffer_drop_older_than(b, 3) == 0);
    assert(vms_arbiter_buffer_size(b) == 20);
    assert(vms_arbiter_buffer_drop_older_than(b, 90) == 20);
    assert(vms_arbiter_buffer_size(b) == 0);

    return 0;
}
