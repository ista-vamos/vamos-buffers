#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "vector.h"

void shm_vector_init(shm_vector *vec, size_t elem_size) {
        memset(vec, 0, sizeof(*vec));
        vec->element_size = elem_size;
}

void shm_vector_destroy(shm_vector *vec) {
        free(vec->data);
}

void *shm_vector_extend(shm_vector *vec) {
    if (vec->size >= vec->alloc_size) {
        // TODO: exp. growth?
        vec->alloc_size += 10;
        vec->data = realloc(vec->data,
                            vec->alloc_size * vec->element_size);
        assert(vec->data != NULL);
    }

    assert(vec->size < vec->alloc_size && "Vector too small");
    void *addr = ((unsigned char *)vec->data) + vec->size*vec->element_size;
    ++vec->size;
    return addr;
}

size_t shm_vector_push(shm_vector *vec, void *elem) {
    memcpy(shm_vector_extend(vec), elem, vec->element_size);
    return vec->size;
}

size_t shm_vector_pop(shm_vector *vec) {
        assert(vec->size > 0);
        return --vec->size;
}

size_t shm_vector_size(shm_vector *vec) {
        return vec->size;
}

/*
 * Return the pointer to the element at index 'idx'
 */
void *shm_vector_at(shm_vector *vec, size_t idx) {
        assert(idx < vec->size);
        return (void*)(((unsigned char *)vec->data) + idx*vec->element_size);
}
