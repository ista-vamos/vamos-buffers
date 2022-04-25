#include <assert.h>
#include "shmbuf/buffer.h"

int main(void) {
    int i;
    struct buffer *b = initialize_shared_buffer(sizeof(int));
    assert(b);
    assert(buffer_size(b) == 0);
    for (i = 1; i < 101; ++i) {
        assert(buffer_push(b, &i, sizeof(int)) == true);
    }
    assert(buffer_size(b) == 100);
    int j;
    for (i = 1; i < 101; ++i) {
        assert(buffer_pop(b, &j) == true);
        assert(j == i);
    }
    assert(buffer_size(b) == 0);

    // pop k
    for (i = 1; i < 101; ++i) {
        assert(buffer_push(b, &i, sizeof(int)) == true);
    }
    assert(buffer_size(b) == 100);
    int num[100];
    assert(buffer_pop_k(b, num, 101) == false);
    assert(buffer_pop_k(b, num, 100) == true);
    for (i = 0; i < 100; ++i) {
        assert(num[i] == i + 1);
    }

    // pop k with rotate
    for (i = 1; i < 1024; ++i) {
        assert(buffer_push(b, &i, sizeof(int)) == true);
    }
    assert(buffer_push(b, &i, sizeof(int)) == false);
    assert(buffer_pop(b, &i) == true && i == 1);
    assert(buffer_pop(b, &i) == true && i == 2);
    assert(buffer_pop(b, &i) == true && i == 3);
    assert(buffer_pop(b, &i) == true && i == 4);
    i = 1024;
    assert(buffer_push(b, &i, sizeof(int)) == true);
    i = 1025;
    assert(buffer_push(b, &i, sizeof(int)) == true);
    // buffer contains 5, ... 1025
    assert(buffer_size(b) == 1021);
    int nums[1021];
    assert(buffer_pop_k(b, nums, 1022) == false);
    assert(buffer_pop_k(b, nums, 1021) == true);
    for (i = 0; i < 1021; ++i) {
        assert(nums[i] == i + 5);
    }

    destroy_shared_buffer(b);
}
