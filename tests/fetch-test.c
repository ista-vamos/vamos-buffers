#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>

#include "shmbuf/buffer-private.h"
#include "vamos-buffers/core/arbiter.h"
#include "vamos-buffers/core/stream.h"
#include "vamos-buffers/shmbuf/buffer.h"

static int stream_ready = 1;
static bool is_ready(vms_stream *s) {
    (void)s;
    return !!stream_ready;
}

struct event {
    vms_event base;
    int n;
};

void *filler_thread(void *data) {
    struct buffer *buffer = (struct buffer *)data;
    struct event ev;
    ev.base.kind = vms_get_last_special_kind() + 1;
    for (int i = 0; i < 4; ++i) {
        ev.base.id = i + 1;
        ev.n = i;
        // printf("PUSH Event: {{%lu, %lu}, %d}\n", ev.base.id, ev.base.kind,
        // ev.n);
        assert(buffer_push(buffer, &ev, sizeof(ev)) == true);
    }
    stream_ready = 0;
    pthread_exit(NULL);
}

int main(void) {
    struct buffer *buffer =
        initialize_local_buffer("/dummy", sizeof(struct event), 30, NULL);
    assert(buffer);
    vms_stream dummy_stream;
    vms_stream *stream = &dummy_stream;

    vms_stream_init(stream, buffer, sizeof(struct event), is_ready, NULL, NULL,
                    NULL, NULL, "dummy-stream", "dummy");

    vms_arbiter_buffer *arbiter_buffer =
        vms_arbiter_buffer_create(stream, sizeof(int), 3);
    vms_arbiter_buffer_set_active(arbiter_buffer, 1);

    pthread_t tid;
    pthread_create(&tid, NULL, filler_thread, buffer);

    struct event *ev;
    for (int i = 0; i < 4; ++i) {
        ev = stream_fetch(stream, arbiter_buffer);
        assert(ev);
        assert(ev->n == i);
        // printf("POP Event: {{%lu, %lu}, %d}\n", ev->base.id, ev->base.kind,
        // ev->n);
        vms_stream_consume(stream, 1);
    }

    ev = stream_fetch(stream, arbiter_buffer);
    assert(!ev);

    pthread_join(tid, NULL);
}
