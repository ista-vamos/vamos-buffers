#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <threads.h>
#include <time.h>

#include "vamos-buffers/core/par_queue.h"
#include "vamos-buffers/core/queue_spsc.h"
#include "vamos-buffers/core/source.h"
#include "vamos-buffers/shmbuf/buffer.h"

#ifdef RMIND_RINGBUF
#include "ringbuf.h"
#endif

#define N 50000000

static double elapsed_time(struct timespec *start, struct timespec *end) {
    long seconds = end->tv_sec - start->tv_sec;
    long nanoseconds = end->tv_nsec - start->tv_nsec;
    return seconds + nanoseconds * 1e-9;
}

static double report_time(const char *msg, struct timespec *start,
                          struct timespec *end) {
    double elapsed = elapsed_time(start, end);
    printf("%s: %lf seconds.\n", msg, elapsed);
    return elapsed;
}

#if 0
static void run_shmbuf_push_pop_st() {
    struct source_control *ctrl = source_control_define(1, "dummy", "i");
    struct buffer *buff = create_shared_buffer("/test", sizeof(int), ctrl);
	assert(buff);

    assert(buffer_capacity(buff) >= N);
    struct timespec start, mid, end;
    double elapsed = 0;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (int i = 0; i < N; ++i) {
        buffer_push(buff, &i, sizeof(int));
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);

    elapsed += report_time("[shmbuf], single-threaded, push", &start, &mid);

    assert(buffer_size(buff) == N);
    int j;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);
    for (int i = 0; i < N; ++i) {
        buffer_pop(buff, &j);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    elapsed += report_time("[shmbuf], single-threaded, pop", &mid, &end);
    printf("\033[34m[shmbuf], single-threaded, push/pop: %lf seconds.\033[0m\n", elapsed);

    assert(buffer_size(buff) == 0);
    destroy_shared_buffer(buff);
}


/* to see if there is any difference between malloce'd and shared memory */
struct buffer *initialize_local_buffer(const char *key, size_t elem_size,
                                       struct source_control *control);
void release_local_buffer(struct buffer *buff);

static void run_local_shmbuf_push_pop_st() {
    struct source_control *ctrl = source_control_define(1, "dummy", "i");
    struct buffer *buff = initialize_local_buffer("/test", sizeof(int), ctrl);

    assert(buff);

    assert(buffer_capacity(buff) >= N);
    struct timespec start, mid, end;
    double elapsed = 0;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (int i = 0; i < N; ++i) {
        buffer_push(buff, &i, sizeof(int));
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);

    elapsed += report_time("[local shmbuf] single-threaded, push", &start, &mid);

    assert(buffer_size(buff) == N);
    int j;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);
    for (int i = 0; i < N; ++i) {
        buffer_pop(buff, &j);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    elapsed += report_time("[local shmbuf], single-threaded, pop", &mid, &end);
    printf("\033[34m[local shmbuf], single-threaded, push/pop: %lf seconds.\033[0m\n", elapsed);

    assert(buffer_size(buff) == 0);
    release_local_buffer(buff);
}
#endif

static void run_par_queue_push_pop_st(void) {
    shm_par_queue q;
    shm_par_queue_init(&q, N, sizeof(int));

    struct timespec start, mid, end;
    double elapsed = 0;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (int i = 0; i < N; ++i) {
        shm_par_queue_push(&q, &i, sizeof(int));
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);

    elapsed += report_time("[par-queue] single-threaded, push", &start, &mid);

    assert(shm_par_queue_size(&q) == N);
    int j;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);
    for (int i = 0; i < N; ++i) {
        shm_par_queue_pop(&q, &j);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    elapsed += report_time("[par-queue], single-threaded, pop", &mid, &end);
    printf(
        "\033[34m[par-queue], single-threaded, push/pop: %lf seconds.\033[0m\n",
        elapsed);

    assert(shm_par_queue_size(&q) == 0);
    shm_par_queue_destroy(&q);
}

static void run_queue_spsc_push_pop_st(void) {
    shm_queue_spsc q;
    shm_queue_spsc_init(&q, N);

    int *buff = malloc(sizeof(int) * N);
    assert(buff);

    int j;
    size_t off;
    struct timespec start, mid, end;
    double elapsed = 0;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (int i = 0; i < N; ++i) {
        shm_queue_spsc_write_offset(&q, &off);
        assert(off == (unsigned)i);
        buff[off] = i;
        shm_queue_spsc_write_finish(&q);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);

    elapsed += report_time("[queue-spsc] single-threaded, push", &start, &mid);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);
    for (int i = 0; i < N; ++i) {
        shm_queue_spsc_read_offset(&q, &off);
        assert(off == (unsigned)i);
        memcpy(&j, buff + i, sizeof(int));
        shm_queue_spsc_consume(&q, 1);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    elapsed += report_time("[queue-spsc], single-threaded, pop", &mid, &end);
    printf(
        "\033[34m[queue-spsc], single-threaded, push/pop: %lf "
        "seconds.\033[0m\n",
        elapsed);

    free(buff);
}

struct thr_data {
    double elapsed;
    void *data_buffer;
    void *ringbuffer;
    size_t writer_waited;
};

static int par_queue_push_pop_1_writer(void *arg) {
    struct thr_data *data = (struct thr_data *)arg;
    shm_par_queue *q = (shm_par_queue *)data->ringbuffer;

    int i;
    struct timespec start, mid;
    int *addr;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (i = 0; i < N; ++i) {
        while (!(addr = shm_par_queue_write_ptr(q))) {
            ++data->writer_waited;
        }
        *addr = i;
        shm_par_queue_write_finish(q);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);

    data->elapsed = elapsed_time(&start, &mid);

    thrd_exit(0);
}

static void run_par_queue_push_pop_1(void) {
    shm_par_queue q;
    shm_par_queue_init(&q, N, sizeof(int));

    thrd_t tid;
    struct thr_data data = {
        .data_buffer = NULL, .ringbuffer = &q, .writer_waited = 0};
    thrd_create(&tid, par_queue_push_pop_1_writer, &data);

    int j;
    unsigned int n = 0;
    size_t reader_waited = 0;
    struct timespec mid, end;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);
    while (n < N) {
        if (shm_par_queue_pop(&q, &j) == false) {
            ++reader_waited;
            continue;
        }
        ++n;
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    thrd_join(tid, NULL);

    printf("[par-queue], writing: %lf\n", data.elapsed);
    double elapsed = report_time("[par-queue], reading", &mid, &end);
    printf("[par-queue], writer waited: %lu, reader waited: %lu\n",
           data.writer_waited, reader_waited);
    printf("\033[34m[par-queue], totally: %lf seconds.\033[0m\n",
           elapsed + data.elapsed);
}

static int queue_spsc_push_pop_1_writer(void *arg) {
    struct thr_data *data = (struct thr_data *)arg;
    shm_queue_spsc *q = (shm_queue_spsc *)data->ringbuffer;
    int *buff = data->data_buffer;

    int i;
    size_t off;
    struct timespec start, mid;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (i = 0; i < N; ++i) {
        while (!shm_queue_spsc_write_offset(q, &off)) {
            ++data->writer_waited;
        }
        buff[off] = i;
        shm_queue_spsc_write_finish(q);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);

    data->elapsed = elapsed_time(&start, &mid);

    thrd_exit(0);
}

static void run_queue_spsc_push_pop_1(void) {
    shm_queue_spsc q;
    shm_queue_spsc_init(&q, N);
    int *buff = malloc(sizeof(int) * N);
    assert(buff);

    thrd_t tid;
    struct thr_data data = {
        .data_buffer = buff, .ringbuffer = &q, .writer_waited = 0};
    thrd_create(&tid, queue_spsc_push_pop_1_writer, &data);

    int j;
    unsigned int n = 0;
    size_t off;
    size_t reader_waited = 0;
    struct timespec mid, end;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);
    while (n < N) {
        if (shm_queue_spsc_read_offset(&q, &off) == 0) {
            ++reader_waited;
            continue;
        }
        memcpy(&j, buff + n, sizeof(int));
        shm_queue_spsc_consume(&q, 1);
        ++n;
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    thrd_join(tid, NULL);

    printf("[queue-spsc], writing: %lf\n", data.elapsed);
    double elapsed = report_time("[queue-spsc], reading", &mid, &end);
    printf("[queue-spsc], writer waited: %lu, reader waited: %lu\n",
           data.writer_waited, reader_waited);
    printf("\033[34m[queue-spsc], totally: %lf seconds.\033[0m\n",
           elapsed + data.elapsed);

    free(buff);
}

#ifdef RMIND_RINGBUF
static void run_rmind_ringbuf_push_pop_st() {
    size_t ringbuf_obj_size;
    ringbuf_get_sizes(1, &ringbuf_obj_size, NULL);
    int *buff = malloc(sizeof(int) * N);
    assert(buff);

    ringbuf_t *r = malloc(ringbuf_obj_size);
    ringbuf_worker_t *w;
    ssize_t off;
    size_t uoff;

    ringbuf_setup(r, 1, N + 1);
    w = ringbuf_register(r, 0);

    struct timespec start, mid, end;
    double elapsed = 0;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (int i = 0; i < N; ++i) {
        off = ringbuf_acquire(r, w, 1);
        // assert(off == i);
        buff[off] = i;
        ringbuf_produce(r, w);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);

    elapsed += report_time("[rmind-rbuf] single-threaded, push", &start, &mid);

    int j;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);
    for (int i = 0; i < N; ++i) {
        off = ringbuf_consume(r, &uoff);
        // assert(uoff == (unsigned)i);
        memcpy(&j, buff + i, sizeof(int));
        ringbuf_release(r, 1);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    elapsed += report_time("[rmind-rbuf], single-threaded, pop", &mid, &end);
    printf(
        "\033[34m[rmind-rbuf], single-threaded, push/pop: %lf "
        "seconds.\033[0m\n",
        elapsed);

    ringbuf_unregister(r, w);
    free(buff);
    free(r);
}

static int ringbuf_push_pop_1_writer(void *arg) {
    struct thr_data *data = (struct thr_data *)arg;
    ringbuf_t *r = (ringbuf_t *)data->ringbuffer;
    int *buff = data->data_buffer;

    ringbuf_worker_t *w = ringbuf_register(r, 0);

    int i;
    ssize_t off;
    struct timespec start, mid;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (i = 0; i < N; ++i) {
        while ((off = ringbuf_acquire(r, w, 1)) == -1) {
            ++data->writer_waited;
        }
        buff[off] = i;
        ringbuf_produce(r, w);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);

    data->elapsed = elapsed_time(&start, &mid);
    ringbuf_unregister(r, w);

    thrd_exit(0);
}

static void run_rmind_ringbuf_push_pop_1() {
    size_t ringbuf_obj_size;
    ringbuf_get_sizes(1, &ringbuf_obj_size, NULL);
    int *buff = malloc(sizeof(int) * N);
    assert(buff);

    ringbuf_t *r = malloc(ringbuf_obj_size);
    ringbuf_setup(r, 1, N + 1);

    ssize_t off;
    size_t uoff;
    int j;
    unsigned int n = 0;
    size_t reader_waited = 0;
    struct timespec mid, end;

    thrd_t tid;
    struct thr_data data = {
        .data_buffer = buff, .ringbuffer = r, .writer_waited = 0};
    thrd_create(&tid, ringbuf_push_pop_1_writer, &data);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);
    while (n < N) {
        if ((off = ringbuf_consume(r, &uoff)) == 0) {
            ++reader_waited;
            continue;
        }
        memcpy(&j, buff + off, sizeof(int));
        ringbuf_release(r, 1);
        ++n;
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    thrd_join(tid, NULL);

    printf("[rmind-ringbuf], writing: %lf\n", data.elapsed);
    double elapsed = report_time("[ringbuf], reading", &mid, &end);
    printf("[rmind-ringbuf], writer waited: %lu, reader waited: %lu\n",
           data.writer_waited, reader_waited);
    printf("\033[34m[rmind-ringbuf], totally: %lf seconds.\033[0m\n",
           elapsed + data.elapsed);

    free(buff);
    free(r);
}
#endif /* RMIND_RINGBUF */

int main(void) {
#ifndef NDEBUG
    printf("\033[31mWARNING: this is not a Release build!\033[0m\n");
#endif

    run_par_queue_push_pop_st();
    puts("----------");
    run_queue_spsc_push_pop_st();
    puts("----------");
#ifdef RMIND_RINGBUF
    run_rmind_ringbuf_push_pop_st();
    puts("----------");
#endif
    run_par_queue_push_pop_1();
    puts("----------");
    run_queue_spsc_push_pop_1();
#ifdef RMIND_RINGBUF
    puts("----------");
    run_rmind_ringbuf_push_pop_1();
#endif
}
