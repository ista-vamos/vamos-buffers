#include <assert.h>
#include <stdlib.h>
#include "shmbuf/buffer.h"

int main(void) {
    size_t i;
    struct buffer *b = initialize_shared_buffer(sizeof(size_t));
    assert(b);
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
    size_t num[100];
    assert(buffer_pop_k(b, num, 101) == false);
    assert(buffer_pop_k(b, num, 100) == true);
    for (i = 0; i < 100; ++i) {
        assert(num[i] == i + 1);
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
    size_t *nums = malloc(sizeof(size_t)*(capacity-2));
    assert(buffer_pop_k(b, nums, capacity - 1) == false);
    assert(buffer_pop_k(b, nums, capacity - 2) == true);
    for (i = 0; i < capacity - 2; ++i) {
        assert(nums[i] == i + 5);
    }

    free(nums);
    destroy_shared_buffer(b);
}
