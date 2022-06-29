#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>

#include "stream.h"
#include "arbiter.h"
#include "shmbuf/buffer.h"

struct buffer *initialize_local_buffer(const char *key,
                                       size_t elem_size,
                                       struct source_control *control);

static int stream_ready = 1;
static bool is_ready(shm_stream *s) {
        (void)s;
        return !!stream_ready;
}

struct event {
        shm_event base;
        int n;
};

void *filler_thread(void *data) {
        struct buffer *buffer = (struct buffer *)data;
        struct event ev;
        ev.base.kind = shm_mk_event_kind("dummy-ev", sizeof(ev), "d");
        for (int i = 0; i < 4; ++i) {
                ev.base.id = i + 1;
                ev.n = i;
                //printf("PUSH Event: {{%lu, %lu}, %d}\n", ev.base.id, ev.base.kind, ev.n);
                assert(buffer_push(buffer, &ev, sizeof(ev)) == true);
        }
        stream_ready = 0;
        pthread_exit(NULL);
}

int main(void) {
        initialize_events();

        struct buffer *buffer
                = initialize_local_buffer("/dummy", sizeof(struct event), NULL);
        assert(buffer);
        shm_stream dummy_stream;
        shm_stream *stream = &dummy_stream;

        shm_stream_init(stream, buffer, sizeof(struct event), is_ready,
                        NULL, NULL, NULL, "dummy-stream");

        shm_arbiter_buffer *arbiter_buffer
                = shm_arbiter_buffer_create(stream, sizeof(int), 3);

        pthread_t tid;
        pthread_create(&tid, NULL, filler_thread, buffer);

        struct event *ev;
        for (int i = 0; i < 4; ++i) {
                ev = stream_fetch(stream, arbiter_buffer);
                assert(ev);
                assert(ev->n == i);
                //printf("POP Event: {{%lu, %lu}, %d}\n", ev->base.id, ev->base.kind, ev->n);
                shm_stream_consume(stream, 1);
        }

        ev = stream_fetch(stream, arbiter_buffer);
        assert(!ev);

        pthread_join(tid, NULL);
}
