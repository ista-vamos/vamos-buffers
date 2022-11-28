#include <assert.h>
// CBMC does not handle C11 threads API
#include "core/par_queue.h"
#include <pthread.h>

int buffer[4];

void *reader(void *data) {
    shm_par_queue *q = (shm_par_queue *)data;
    int            n = 1;
    while (n < 4) {
        if (shm_par_queue_pop(q, &buffer[n])) {
            ++n;
        }
    }
    pthread_exit(0);
}

void *writer(void *data) {
    shm_par_queue *q = (shm_par_queue *)data;
    for (int i = 1; i < 4; ++i) {
        assert(shm_par_queue_push(q, &i, sizeof(i)));
    }

    pthread_exit(0);
}

int main(void) {
    shm_par_queue q;
    shm_par_queue_init(&q, 3, sizeof(int));

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
