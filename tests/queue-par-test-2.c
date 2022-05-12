#include <assert.h>
#include <stdio.h>
#include "parallel_queue.h"

int main(void) {
    int i;
    int num[] = {0, 1, 2, 3, 4, 5, 6};
    shm_par_queue q;
    shm_par_queue_init(&q, 3, sizeof(int));

    assert(shm_par_queue_push_k(&q, num + 1, 5) == 2);
    for (i = 0; i < (int)shm_par_queue_size(&q); ++i) {
        int *p = (int*)shm_par_queue_peek_at(&q, i);
        assert(*p == i + 1);
    }
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
    assert(shm_par_queue_size(&q) == 0);
    assert(shm_par_queue_push_k(&q, num + 2, 5) == 2);
    int *d1, *d2;
    size_t s1, s2;
    size_t n = shm_par_queue_peek(&q, 5, (void**)&d1, &s1, (void**)&d2, &s2);
    assert(n == 3);
    assert(s1 == 2);
    assert(*d1 == 2);
    assert(*(d1+1) == 3);
    assert(s2 == 1);
    assert(*d2 == 4);
    assert(s1 + s2 == n);
    assert(*(int*)shm_par_queue_peek_at(&q, 0) == 2);
    assert(*(int*)shm_par_queue_peek_at(&q, 1) == 3);
    assert(*(int*)shm_par_queue_peek_at(&q, 2) == 4);
    size_t k = 0;
    assert(*(int*)shm_par_queue_peek_atmost_at(&q, &k) == 2 && k == 0);
    k = 1;
    assert(*(int*)shm_par_queue_peek_atmost_at(&q, &k) == 3 && k == 1);
    k = 2;
    assert(*(int*)shm_par_queue_peek_atmost_at(&q, &k) == 4 && k == 2);
    k = 3;
    assert(*(int*)shm_par_queue_peek_atmost_at(&q, &k) == 4 && k == 2);
    k = 4;
    assert(*(int*)shm_par_queue_peek_atmost_at(&q, &k) == 4 && k == 2);
    k = 4444;
    assert(*(int*)shm_par_queue_peek_atmost_at(&q, &k) == 4 && k == 2);
    k = 111111;
    assert(*(int*)shm_par_queue_peek_atmost_at(&q, &k) == 4 && k == 2);
    k = ~(0LL);
    assert(*(int*)shm_par_queue_peek_atmost_at(&q, &k) == 4 && k == 2);

    shm_par_queue_destroy(&q);
}
