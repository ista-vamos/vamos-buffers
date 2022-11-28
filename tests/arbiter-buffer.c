#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "arbiter.h"
#include "shmbuf/buffer.h"
#include "stream.h"

static bool is_ready(shm_stream *s) {
    (void)s;
    return false;
}

struct event {
    shm_event base;
    int       i;
};

struct buffer *initialize_local_buffer(const char *key, size_t elem_size,
                                       struct source_control *control);

int main(void) {
    struct buffer *lbuffer =
        initialize_local_buffer("/dummy", sizeof(struct event), NULL);
    assert(lbuffer);

    shm_stream  dummy_stream;
    shm_stream *stream = &dummy_stream;

    shm_stream_init(stream, lbuffer, sizeof(struct event), is_ready, NULL, NULL,
                    NULL, NULL, "dummy-stream", "dummy");

    shm_arbiter_buffer *b =
        shm_arbiter_buffer_create(stream, sizeof(struct event), 20);
    shm_arbiter_buffer_set_active(b, true);

    struct event ev;
    for (int i = 1; i < 21; ++i) {
        ev.base.id = i;
        ev.i       = i;
        shm_arbiter_buffer_push(b, &ev, sizeof(struct event));
    }

    assert(shm_arbiter_buffer_size(b) == 20);
    assert(shm_arbiter_buffer_drop_older_than(b, 10) == 10);
    assert(shm_arbiter_buffer_size(b) == 10);
    assert(shm_arbiter_buffer_drop(b, 20) == 10);
    assert(shm_arbiter_buffer_size(b) == 0);

    for (int i = 1; i < 21; ++i) {
        ev.base.id = 20 + 2 * i;
        ev.i       = i;
        shm_arbiter_buffer_push(b, &ev, sizeof(struct event));
    }

    assert(shm_arbiter_buffer_size(b) == 20);
    assert(shm_arbiter_buffer_drop_older_than(b, 39) == 9);
    assert(shm_arbiter_buffer_size(b) == 11);
    assert(shm_arbiter_buffer_drop_older_than(b, 58) == 10);
    assert(shm_arbiter_buffer_size(b) == 1);
    assert(shm_arbiter_buffer_drop_older_than(b, 70) == 1);
    assert(shm_arbiter_buffer_size(b) == 0);

    for (int i = 1; i < 21; ++i) {
        ev.base.id = 40 + 2 * i;
        ev.i       = i;
        shm_arbiter_buffer_push(b, &ev, sizeof(struct event));
    }

    assert(shm_arbiter_buffer_drop_older_than(b, 3) == 0);
    assert(shm_arbiter_buffer_size(b) == 20);
    assert(shm_arbiter_buffer_drop_older_than(b, 90) == 20);
    assert(shm_arbiter_buffer_size(b) == 0);

    return 0;
}
