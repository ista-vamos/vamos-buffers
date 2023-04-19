#undef NDEBUG
#include "queue.h"

#include <assert.h>

int main(void) {
    int i;
    int num[] = {0, 1, 2, 3};
    vms_queue q;
    vms_queue_init(&q, 3, sizeof(int));

    assert(vms_queue_pop(&q, &i) == false);
    assert(vms_queue_pop(&q, &i) == false);
    assert(vms_queue_push(&q, &num[1]) == true);
    assert(vms_queue_push(&q, &num[2]) == true);
    assert(vms_queue_push(&q, &num[3]) == true);
    assert(vms_queue_push(&q, &num[3]) == false);
    assert(vms_queue_pop(&q, &i) == true && i == 1);
    assert(vms_queue_push(&q, &num[3]) == true);
    assert(vms_queue_pop(&q, &i) == true && i == 2);
    assert(vms_queue_pop(&q, &i) == true && i == 3);
    assert(vms_queue_pop(&q, &i) == true && i == 3);

    vms_queue_destroy(&q);
}
