#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "core/source.h"
#include "shmbuf/buffer.h"
#include "core/par_queue.h"
#include "core/queue_spsc.h"

#ifdef RMIND_RINGBUF
#include "ringbuf.h"
#endif

#define N 50000

static double report_time(const char *msg, struct timespec *start, struct timespec *end) {
    long seconds = end->tv_sec - start->tv_sec;
    long nanoseconds = end->tv_nsec - start->tv_nsec;
    double elapsed = seconds + nanoseconds*1e-9;
    printf("%s: %lf seconds.\n", msg, elapsed);
    return elapsed;
}

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


static void run_par_queue_push_pop_st() {
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
    printf("\033[34m[par-queue], single-threaded, push/pop: %lf seconds.\033[0m\n", elapsed);

    assert(shm_par_queue_size(&q) == 0);
    shm_par_queue_destroy(&q);
}

static void run_queue_spsc_push_pop_st() {
    shm_queue_spsc q;
    shm_queue_spsc_init(&q, N);

    int *buff = malloc(sizeof(int)*N);
    assert(buff);

    size_t off;
    struct timespec start, mid, end;
    double elapsed = 0;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (int i = 0; i < N; ++i) {
        shm_queue_spsc_write_offset(&q, &off);
        assert(off == (unsigned)i);
        buff[off] = i;
        memcpy(buff + off, &i, sizeof(int));
        shm_queue_spsc_write_finish(&q);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);

    elapsed += report_time("[queue-spsc] single-threaded, push", &start, &mid);

    int j;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);
    for (int i = 0; i < N; ++i) {
        shm_queue_spsc_read_offset(&q, &off);
        assert(off == (unsigned)i);
        memcpy(&j, buff + i, sizeof(int));
        shm_queue_spsc_consume(&q, 1);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    elapsed += report_time("[queue-spsc], single-threaded, pop", &mid, &end);
    printf("\033[34m[queue-spsc], single-threaded, push/pop: %lf seconds.\033[0m\n", elapsed);

    free(buff);
}

#ifdef RMIND_RINGBUF
static void run_rmind_ringbuf_push_pop_st() {
    size_t ringbuf_obj_size;
    ringbuf_get_sizes(1, &ringbuf_obj_size, NULL);
    int *buff = malloc(sizeof(int)*N);
    assert(buff);

    ringbuf_t *r = malloc(ringbuf_obj_size);
    ringbuf_worker_t *w;
    ssize_t off;
    size_t uoff;

    ringbuf_setup(r, 1, N+1);
    w = ringbuf_register(r, 0);

    struct timespec start, mid, end;
    double elapsed = 0;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for (int i = 0; i < N; ++i) {
        off = ringbuf_acquire(r, w, 1);
        //assert(off == i);
        buff[off] = i;
        memcpy(buff + off, &i, sizeof(int));
        ringbuf_produce(r, w);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);

    elapsed += report_time("[rmind-rbuf] single-threaded, push", &start, &mid);

    int j;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &mid);
    for (int i = 0; i < N; ++i) {
        off = ringbuf_consume(r, &uoff);
        //assert(uoff == (unsigned)i);
        memcpy(&j, buff + i, sizeof(int));
        ringbuf_release(r, 1);
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    elapsed += report_time("[rmind-rbuf], single-threaded, pop", &mid, &end);
    printf("\033[34m[rmind-rbuf], single-threaded, push/pop: %lf seconds.\033[0m\n", elapsed);

    ringbuf_unregister(r, w);
    free(buff);
    free(r);
}
#endif /* RMIND_RINGBUF */

int main(void) {
    run_shmbuf_push_pop_st();
    puts("----------");
    run_local_shmbuf_push_pop_st();
    puts("----------");
    run_par_queue_push_pop_st();
    puts("----------");
    run_queue_spsc_push_pop_st();
#ifdef RMIND_RINGBUF
    puts("----------");
    run_rmind_ringbuf_push_pop_st();
#endif
}
