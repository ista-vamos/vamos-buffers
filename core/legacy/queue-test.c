#undef NDEBUG
#include "queue.h"
#include <assert.h>

int main(void) {
    int       i;
    int       num[] = {0, 1, 2, 3};
    shm_queue q;
    shm_queue_init(&q, 3, sizeof(int));

    assert(shm_queue_pop(&q, &i) == false);
    assert(shm_queue_pop(&q, &i) == false);
    assert(shm_queue_push(&q, &num[1]) == true);
    assert(shm_queue_push(&q, &num[2]) == true);
    assert(shm_queue_push(&q, &num[3]) == true);
    assert(shm_queue_push(&q, &num[3]) == false);
    assert(shm_queue_pop(&q, &i) == true && i == 1);
    assert(shm_queue_push(&q, &num[3]) == true);
    assert(shm_queue_pop(&q, &i) == true && i == 2);
    assert(shm_queue_pop(&q, &i) == true && i == 3);
    assert(shm_queue_pop(&q, &i) == true && i == 3);

    shm_queue_destroy(&q);
}
