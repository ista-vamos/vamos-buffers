#include <assert.h>
#include <threads.h>
#include "parallel_queue.h"

int buffer[4];

int reader(void *data) {
    shm_par_queue *q = (shm_par_queue*)data;
    int n = 0;
    while (n < 3) {
        if (shm_par_queue_pop(q, &buffer[n+1])) {
            ++n;
        }
    }
    thrd_exit(0);
}

int writer(void *data) {
    int num[] = {0, 1, 2, 3};
    shm_par_queue *q = (shm_par_queue*)data;
    for (int i = 1; i < 4; ++i)
        shm_par_queue_push(q, &num[i]);

    thrd_exit(0);
}

int main(void) {
    shm_par_queue q;
    shm_par_queue_init(&q, 3, sizeof(int));

    thrd_t r, w;
    thrd_create(&r, reader, &q);
    thrd_create(&w, writer, &q);
    thrd_join(w, NULL);
    thrd_join(r, NULL);

    for (int i = 0; i < 4; ++i) {
        assert(buffer[i] == i);
    }

    shm_par_queue_destroy(&q);
}
