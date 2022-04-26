#include <assert.h>
#include <stdio.h>
#include "parallel_queue.h"

int main(void) {
    int i;
    int num[] = {0, 1, 2, 3, 4, 5, 6};
    shm_par_queue q;
    shm_par_queue_init(&q, 3, sizeof(int));

    assert(shm_par_queue_push_k(&q, num + 1, 5) == 2);
    assert(shm_par_queue_pop(&q, &i) == true && i == 1);
    assert(shm_par_queue_pop(&q, &i) == true && i == 2);
    assert(shm_par_queue_pop(&q, &i) == true && i == 3);
    assert(shm_par_queue_size(&q) == 0);
    assert(shm_par_queue_push(&q, num + 0 , sizeof(i)) == true);
    assert(shm_par_queue_push(&q, num + 1 , sizeof(i)) == true);
    assert(shm_par_queue_pop(&q, &i) == true && i == 0);
    assert(shm_par_queue_push_k(&q, num + 2, 5) == 3);
    assert(shm_par_queue_pop(&q, &i) == true && i == 1);
    assert(shm_par_queue_pop(&q, &i) == true && i == 2);
    assert(shm_par_queue_pop(&q, &i) == true && i == 3);
    shm_par_queue_destroy(&q);
}
