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

struct buffer *initialize_local_buffer(const char *key, size_t elem_size,
                                       struct source_control *control);

static int  stream_ready  = 1;
static int  main_finished = 0;
static bool is_ready(shm_stream *s) {
    (void)s;
    return !!stream_ready;
}

struct event {
    shm_event base;
    size_t    n;
};

static size_t failed_push = 0;
void         *filler_thread(void *data) {
            struct buffer *buffer = (struct buffer *)data;
            struct event   ev;
            ev.base.kind = shm_get_last_special_kind() + 1;
            for (size_t i = 0; i < 10000; ++i) {
                ev.base.id = i + 1;
                ev.n       = i;
                // printf("PUSH Event: {{%lu, %lu}, %lu}\n", ev.base.id, ev.base.kind,
                // ev.n);
                while (!buffer_push(buffer, &ev, sizeof(ev)))
            ++failed_push;
    }
            stream_ready = 0;
            pthread_exit(NULL);
}

void *reader_thread(void *data) {
    shm_arbiter_buffer *arbiter_buffer_r = (shm_arbiter_buffer *)data;
    struct event       *ev;
    size_t              num;
    size_t              last_id = 0;
    size_t              total   = 0;
    while (1) {
        num = shm_arbiter_buffer_peek1(arbiter_buffer_r, (void **)&ev);
        if (num > 0) {
            assert(!shm_event_is_hole((shm_event *)ev));
            assert(ev->n == last_id);
            assert(++last_id == shm_event_id((shm_event *)ev));
            ++total;
            /*
            printf("Abuf Event: {{%lu, %lu}, %lu}\n",
                    ev->base.id, ev->base.kind, ev->n);
            */
            assert(shm_arbiter_buffer_drop(arbiter_buffer_r, 1) == 1);
        }
        if (stream_ready == 0 && main_finished == 1 &&
            shm_arbiter_buffer_size(arbiter_buffer_r) == 0)
            break;
    }

    printf("Totally received %lu events in arbiter buffer\n", total);
    printf("Last id: %lu\n", last_id);
    pthread_exit(NULL);
}

static size_t waiting_for_arbbuf = 0;

int main(void) {
    struct buffer *buffer =
        initialize_local_buffer("/dummy", sizeof(struct event), NULL);
    assert(buffer);
    shm_stream  dummy_stream;
    shm_stream *stream = &dummy_stream;

    shm_stream_init(stream, buffer, sizeof(struct event), is_ready, NULL, NULL,
                    NULL, NULL, "dummy-stream", "dummy");

    /* into this buffer, stream_fetch() will push dropped() events if any */
    shm_arbiter_buffer *arbiter_buffer =
        shm_arbiter_buffer_create(stream, sizeof(struct event), 10);

    /* into this buffer, we will push events here and read them */
    shm_arbiter_buffer *arbiter_buffer_r =
        shm_arbiter_buffer_create(stream, sizeof(struct event), 3);

    shm_arbiter_buffer_set_active(arbiter_buffer, 1);
    shm_arbiter_buffer_set_active(arbiter_buffer_r, 1);

    pthread_t tid, tida;
    pthread_create(&tid, NULL, filler_thread, buffer);
    pthread_create(&tida, NULL, reader_thread, arbiter_buffer_r);

    struct event *ev;
    struct event *out;
    for (size_t i = 0; i < 10000; ++i) {
        ev = stream_fetch(stream, arbiter_buffer);
        /* there should be no dropped event generated
         * since we are outputing the events into another buffer */
        assert(shm_arbiter_buffer_size(arbiter_buffer) == 0);

        assert(ev);
        assert(ev->n == i);
        assert(ev->base.id == i + 1);
        // printf("POP Event: {{%lu, %lu}, %lu}\n", ev->base.id, ev->base.kind,
        // ev->n);

        /* this function also automatically drops events */
        while (!(out = shm_arbiter_buffer_write_ptr(arbiter_buffer_r))) {
            ++waiting_for_arbbuf;
        }

        *out = *ev;
        shm_arbiter_buffer_write_finish(arbiter_buffer_r);

        shm_stream_consume(stream, 1);
    }

    ev = stream_fetch(stream, arbiter_buffer);
    assert(!ev);

    main_finished = 1;

    pthread_join(tid, NULL);
    pthread_join(tida, NULL);

    printf("Waited on push from source: %lu\n", failed_push);
    printf("Waited on push to arbiter buf: %lu\n", waiting_for_arbbuf);
}
