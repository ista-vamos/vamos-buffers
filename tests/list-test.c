#undef NDEBUG
#include "vamos-buffers/core/list.h"

#include <assert.h>

int main(void) {
    int num[] = {0, 1, 2, 3};
    vms_list l;
    vms_list_init(&l);
    assert(vms_list_size(&l) == 0);
    assert(vms_list_append(&l, num + 0) == 1);
    assert(vms_list_size(&l) == 1);
    assert(vms_list_remove(&l, l.first) == 0);
    assert(vms_list_size(&l) == 0);
    assert(vms_list_prepend(&l, num + 0) == 1);
    assert(vms_list_size(&l) == 1);
    assert(vms_list_remove(&l, l.last) == 0);
    assert(vms_list_size(&l) == 0);

    for (size_t i = 0; i < 4; ++i)
        assert(vms_list_append(&l, num + i) == i + 1);

    vms_list_elem *cur = l.first;
    int i = 0;
    int sum = 0;
    vms_list_elem *x = NULL;
    while (cur) {
        assert(cur->data == num + i);
        if (i == 2)
            x = cur;
        sum += *((int *)cur->data);
        ++i;
        cur = cur->next;
    }
    assert(sum == 6);

    assert(vms_list_remove(&l, x) == 3);
    cur = l.first;
    while (cur) {
        sum += *((int *)cur->data);
        cur = cur->next;
    }
    assert(sum == 10);
    // TODO: we leak it, destroy assumes that the list is dynamically allocated
    // vms_list_destroy(&l, NULL);

    vms_list_init(&l);
    for (size_t i = 0; i < 4; ++i)
        assert(vms_list_append(&l, num + i) == i + 1);
    for (size_t i = 0; i < 4; ++i)
        assert(vms_list_remove(&l, vms_list_last(&l)) == 3 - i);
    assert(vms_list_size(&l) == 0);
    for (size_t i = 0; i < 4; ++i)
        assert(vms_list_append(&l, num + i) == i + 1);
    for (size_t i = 0; i < 4; ++i)
        assert(vms_list_remove(&l, vms_list_first(&l)) == 3 - i);
    assert(vms_list_size(&l) == 0);
    for (size_t i = 0; i < 4; ++i)
        assert(vms_list_append(&l, num + i) == i + 1);
    assert(vms_list_remove(&l, vms_list_first(&l)->next->next) == 3);
    assert(vms_list_remove(&l, vms_list_first(&l)->next->next) == 2);
    assert(vms_list_last(&l) == vms_list_first(&l)->next);
    assert(vms_list_remove(&l, vms_list_first(&l)->next) == 1);
    assert(vms_list_last(&l) == vms_list_first(&l));
}
