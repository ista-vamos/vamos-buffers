#include <assert.h>
// CBMC does not handle C11 threads API
#include <pthread.h>

#include "vamos-buffers/core/par_queue.h"

int buffer[4];

void *reader(void *data) {
    shm_par_queue *q = (shm_par_queue *)data;
    int n = 0;
    while (n < 3) {
        if (shm_par_queue_pop(q, &buffer[n + 1])) {
            ++n;
        }
    }
    pthread_exit(0);
}

void *writer(void *data) {
    int num[] = {0, 1, 2, 3};
    shm_par_queue *q = (shm_par_queue *)data;
    int n = 0;
    while (n < 3) {
        if (shm_par_queue_push(q, &num[n + 1], sizeof(int))) {
            ++n;
        }
    }

    pthread_exit(0);
}

int main(void) {
    shm_par_queue q;
    shm_par_queue_init(&q, 1, sizeof(int));

    pthread_t r, w;
    pthread_create(&r, NULL, reader, &q);
    pthread_create(&w, NULL, writer, &q);
    pthread_join(w, NULL);
    pthread_join(r, NULL);

    for (int i = 1; i < 4; ++i) {
        assert(buffer[i] == i);
    }

    // CBMC does not handle calls to free() with threads...
    // shm_par_queue_destroy(&q);
}
