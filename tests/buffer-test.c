#undef NDEBUG
#include "shmbuf/buffer.h"

#include <assert.h>
#include <stdlib.h>

#include "source.h"

int main(void) {
    size_t i;
    const size_t ctrl_size = sizeof(size_t) + sizeof(struct event_record);
    struct source_control *ctrl = malloc(ctrl_size);
    ctrl->size = ctrl_size;
    ctrl->events[0].size = sizeof(size_t);
    ctrl->events[0].kind = 2;
    ctrl->events[0].name[0] = '\0';
    ctrl->events[0].signature[0] = '\0';

    struct buffer *b = create_shared_buffer("/testkey", 128, ctrl);
    assert(b);
    assert(buffer_elem_size(b) >= sizeof(size_t));
    free(ctrl);

    assert(buffer_size(b) == 0);
    for (i = 1; i < 101; ++i) {
        assert(buffer_push(b, &i, sizeof(size_t)) == true);
    }
    assert(buffer_size(b) == 100);
    size_t j;
    for (i = 1; i < 101; ++i) {
        assert(buffer_pop(b, &j) == true);
        assert(j == i);
    }
    assert(buffer_size(b) == 0);

    // pop k
    for (i = 1; i < 101; ++i) {
        assert(buffer_push(b, &i, sizeof(size_t)) == true);
    }
    assert(buffer_size(b) == 100);
    size_t num;
    for (i = 0; i < 100; ++i) {
        assert(buffer_pop(b, &num) == true);
        assert(num == i + 1);
    }

    // pop k with rotate
    const size_t capacity = buffer_capacity(b);
    for (i = 1; i < capacity + 1; ++i) {
        assert(buffer_push(b, &i, sizeof(size_t)) == true);
    }
    assert(buffer_push(b, &i, sizeof(size_t)) == false);
    assert(buffer_pop(b, &i) == true && i == 1);
    assert(buffer_pop(b, &i) == true && i == 2);
    assert(buffer_pop(b, &i) == true && i == 3);
    assert(buffer_pop(b, &i) == true && i == 4);
    i = capacity + 1;
    assert(buffer_push(b, &i, sizeof(size_t)) == true);
    i = capacity + 2;
    assert(buffer_push(b, &i, sizeof(size_t)) == true);
    // buffer contains 5, ... capacity + 1
    assert(buffer_size(b) == capacity - 2);
    destroy_shared_buffer(b);
}
