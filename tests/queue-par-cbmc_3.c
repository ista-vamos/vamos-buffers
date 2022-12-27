#include <assert.h>

#include "core/par_queue.h"

extern bool nondet(void);

#define CAPACITY 2
int main(void) {
    shm_par_queue q;
    shm_par_queue_init(&q, CAPACITY, sizeof(int));

    size_t elem_num = 0;
    int x = 0xbee;
    while (1) {
        if (nondet()) {
            /* POP */
            bool ret = shm_par_queue_pop(&q, &x);
            if (ret) {
                assert(elem_num > 0);
                --elem_num;
                assert(shm_par_queue_size(&q) < CAPACITY);
                assert(x == 0xbee);
            } else {
                assert(elem_num == 0);
                assert(shm_par_queue_size(&q) == 0);
            }
        } else {
            /* PUSH */
            if (shm_par_queue_push(&q, &x, sizeof(int))) {
                ++elem_num;
                assert(elem_num <= CAPACITY);
                assert(shm_par_queue_size(&q) <= CAPACITY);
            } else {
                assert(elem_num == CAPACITY);
                assert(shm_par_queue_size(&q) == CAPACITY);
                assert(shm_par_queue_size(&q) == shm_par_queue_capacity(&q));
            }
            assert(shm_par_queue_size(&q) > 0);
        }
    }

    shm_par_queue_destroy(&q);
}
