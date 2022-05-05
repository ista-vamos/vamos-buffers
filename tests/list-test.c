#include <assert.h>
#include "list.h"

int main(void) {
    int num[] = {0, 1, 2, 3};
    shm_list l;
    shm_list_init(&l);
    assert(shm_list_size(&l) == 0);
    assert(shm_list_append(&l, num + 0) == 1);
    assert(shm_list_size(&l) == 1);
    assert(shm_list_remove(&l, l.first) == 0);
    assert(shm_list_size(&l) == 0);
    assert(shm_list_prepend(&l, num + 0) == 1);
    assert(shm_list_size(&l) == 1);
    assert(shm_list_remove(&l, l.last) == 0);
    assert(shm_list_size(&l) == 0);

    for (size_t i = 0; i < 4; ++i)
        assert(shm_list_append(&l, num + i) == i+1);

    shm_list_elem *cur = l.first;
    int i = 0;
    int sum = 0;
    shm_list_elem *x;
    while (cur) {
            assert(cur->data == num + i);
            if (i == 2)
                    x = cur;
            sum += *((int*)cur->data);
            ++i;
            cur = cur->next;
    }
    assert(sum == 6);

    assert(shm_list_remove(&l, x) == 3);
    cur = l.first;
    while (cur) {
            sum += *((int*)cur->data);
            cur = cur->next;
    }
    assert(sum == 10);
    shm_list_destroy(&l, NULL);

    shm_list_init(&l);
    for (size_t i = 0; i < 4; ++i)
        assert(shm_list_append(&l, num + i) == i+1);
    for (size_t i = 0; i < 4; ++i)
        assert(shm_list_remove(&l, shm_list_last(&l)) == 3 - i);
    assert(shm_list_size(&l) == 0);
    for (size_t i = 0; i < 4; ++i)
        assert(shm_list_append(&l, num + i) == i+1);
    for (size_t i = 0; i < 4; ++i)
        assert(shm_list_remove(&l, shm_list_first(&l)) == 3 - i);
    assert(shm_list_size(&l) == 0);
    for (size_t i = 0; i < 4; ++i)
        assert(shm_list_append(&l, num + i) == i+1);
    assert(shm_list_remove(&l, shm_list_first(&l)->next->next) == 3);
    assert(shm_list_remove(&l, shm_list_first(&l)->next->next) == 2);
    assert(shm_list_last(&l) == shm_list_first(&l)->next);
    assert(shm_list_remove(&l, shm_list_first(&l)->next) == 1);
    assert(shm_list_last(&l) == shm_list_first(&l));
}
