#undef NDEBUG
#include <assert.h>
#include <stdio.h>

#include "vamos-buffers/core/par_queue.h"

int main(void) {
    size_t i;
    int *p;
    int num[] = {0, 1, 2, 3, 4, 5, 6};
    shm_par_queue q;
    shm_par_queue_init(&q, 3, sizeof(int));
    assert(shm_par_queue_size(&q) == 0);
    assert(shm_par_queue_free_num(&q) == 3);
    assert(shm_par_queue_peek_at(&q, 2) == NULL);
    i = 2;
    assert(shm_par_queue_peek_atmost_at(&q, &i) == NULL);
    assert(i == 0);

    assert(shm_par_queue_push(&q, num + 1, sizeof(int)) == true);
    assert(shm_par_queue_free_num(&q) == 2);
    assert(shm_par_queue_size(&q) == 1);
    assert(shm_par_queue_peek_at(&q, 2) == NULL);
    i = 2;
    assert((p = (int *)shm_par_queue_peek_atmost_at(&q, &i)) != NULL);
    assert(i == 0);
    assert(*p == 1);

    assert(shm_par_queue_push(&q, num + 2, sizeof(int)) == true);
    assert(shm_par_queue_free_num(&q) == 1);
    assert(shm_par_queue_size(&q) == 2);
    assert(shm_par_queue_peek_at(&q, 2) == NULL);
    i = 2;
    assert((p = (int *)shm_par_queue_peek_atmost_at(&q, &i)) != NULL);
    assert(i == 1);
    assert(*p == 2);

    assert(shm_par_queue_push(&q, num + 3, sizeof(int)) == true);
    assert(shm_par_queue_free_num(&q) == 0);
    assert(shm_par_queue_size(&q) == 3);
    assert((p = (int *)shm_par_queue_peek_at(&q, 2)) != NULL);
    assert(*p == 3);
    i = 2;
    assert((p = (int *)shm_par_queue_peek_atmost_at(&q, &i)) != NULL);
    assert(i == 2);
    assert(*p == 3);
    i = 5;
    assert((p = (int *)shm_par_queue_peek_atmost_at(&q, &i)) != NULL);
    assert(i == 2);
    assert(*p == 3);
    i = 0;
    assert((p = (int *)shm_par_queue_peek_atmost_at(&q, &i)) != NULL);
    assert(i == 0);
    assert(*p == 1);
    assert((p = (int *)shm_par_queue_peek_at(&q, 0)) != NULL);
    assert(*p == 1);
    i = 1;
    assert((p = (int *)shm_par_queue_peek_atmost_at(&q, &i)) != NULL);
    assert(i == 1);
    assert(*p == 2);
    assert((p = (int *)shm_par_queue_peek_at(&q, 1)) != NULL);
    assert(*p == 2);

    assert(shm_par_queue_push(&q, num + 4, sizeof(int)) == false);
    assert(shm_par_queue_free_num(&q) == 0);
    assert(shm_par_queue_size(&q) == 3);
    for (i = 0; i < shm_par_queue_size(&q); ++i) {
        size_t j = i;
        p = (int *)shm_par_queue_peek_at(&q, i);
        assert(*p == (int)i + 1);
        p = (int *)shm_par_queue_peek_atmost_at(&q, &j);
        assert(j == i);
        assert(*p == (int)i + 1);
    }
    assert(shm_par_queue_pop(&q, &i) == true && i == 1);
    assert(shm_par_queue_size(&q) == 2);
    assert(shm_par_queue_free_num(&q) == 1);
    assert(shm_par_queue_pop(&q, &i) == true && i == 2);
    assert(shm_par_queue_size(&q) == 1);
    assert(shm_par_queue_free_num(&q) == 2);

    assert((p = (int *)shm_par_queue_peek_at(&q, 1)) == NULL);
    i = 1;
    assert((p = (int *)shm_par_queue_peek_atmost_at(&q, &i)) != NULL);
    assert(i == 0);
    assert(*p == 3);

    assert((p = (int *)shm_par_queue_peek_at(&q, 0)) != NULL);
    assert(*p == 3);
    i = 0;
    assert((p = (int *)shm_par_queue_peek_atmost_at(&q, &i)) != NULL);
    assert(i == 0);
    assert(*p == 3);
    i = 2;
    assert((p = (int *)shm_par_queue_peek_atmost_at(&q, &i)) != NULL);
    assert(i == 0);
    assert(*p == 3);

    assert(shm_par_queue_pop(&q, &i) == true && i == 3);
    assert(shm_par_queue_free_num(&q) == 3);
    assert(shm_par_queue_size(&q) == 0);
    assert((p = (int *)shm_par_queue_peek_at(&q, 1)) == NULL);
    i = 1;
    assert((p = (int *)shm_par_queue_peek_atmost_at(&q, &i)) == NULL);
    assert(i == 0);
    assert((p = (int *)shm_par_queue_peek_at(&q, 0)) == NULL);
    i = 0;
    assert((p = (int *)shm_par_queue_peek_atmost_at(&q, &i)) == NULL);
    assert(i == 0);

    assert(shm_par_queue_push(&q, num + 0, sizeof(int)) == true);
    assert(shm_par_queue_push(&q, num + 1, sizeof(int)) == true);
    assert(shm_par_queue_pop(&q, &i) == true && i == 0);
    assert(shm_par_queue_push(&q, num + 2, sizeof(int)) == true);
    assert(shm_par_queue_push(&q, num + 3, sizeof(int)) == true);
    assert(shm_par_queue_push(&q, num + 4, sizeof(int)) == false);
    assert(shm_par_queue_pop(&q, &i) == true && i == 1);
    assert(shm_par_queue_pop(&q, &i) == true && i == 2);
    assert(shm_par_queue_pop(&q, &i) == true && i == 3);
    assert(shm_par_queue_size(&q) == 0);
    assert(shm_par_queue_push(&q, num + 2, sizeof(int)) == true);
    assert(shm_par_queue_push(&q, num + 3, sizeof(int)) == true);
    assert(shm_par_queue_push(&q, num + 4, sizeof(int)) == true);
    assert(shm_par_queue_push(&q, num + 5, sizeof(int)) == false);
    int *d1, *d2;
    size_t s1, s2;
    size_t n = shm_par_queue_peek(&q, 5, (void **)&d1, &s1, (void **)&d2, &s2);
    assert(n == 3);
    assert(s1 + s2 == 3);
    size_t si = 0;
    while (si < s1) {
        assert(d1[si] == (int)(si + 2));
        ++si;
    }
    si = 0;
    while (si < s2) {
        assert(d2[si] == (int)(si + 2 + s1));
        ++si;
    }

    assert(*(int *)shm_par_queue_peek_at(&q, 0) == 2);
    assert(*(int *)shm_par_queue_peek_at(&q, 1) == 3);
    assert(*(int *)shm_par_queue_peek_at(&q, 2) == 4);
    size_t k = 0;
    assert(*(int *)shm_par_queue_peek_atmost_at(&q, &k) == 2 && k == 0);
    k = 1;
    assert(*(int *)shm_par_queue_peek_atmost_at(&q, &k) == 3 && k == 1);
    k = 2;
    assert(*(int *)shm_par_queue_peek_atmost_at(&q, &k) == 4 && k == 2);
    k = 3;
    assert(*(int *)shm_par_queue_peek_atmost_at(&q, &k) == 4 && k == 2);
    k = 4;
    assert(*(int *)shm_par_queue_peek_atmost_at(&q, &k) == 4 && k == 2);
    k = 4444;
    assert(*(int *)shm_par_queue_peek_atmost_at(&q, &k) == 4 && k == 2);
    k = 111111;
    assert(*(int *)shm_par_queue_peek_atmost_at(&q, &k) == 4 && k == 2);
    k = ~(0LL);
    assert(*(int *)shm_par_queue_peek_atmost_at(&q, &k) == 4 && k == 2);

    shm_par_queue_destroy(&q);
}
