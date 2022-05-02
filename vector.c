#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "vector.h"

void shm_vector_init(shm_vector *vec, size_t elem_size) {
        assert(elem_size > 0);
        memset(vec, 0, sizeof(*vec));
        vec->element_size = elem_size;
}

void shm_vector_destroy(shm_vector *vec) {
        free(vec->data);
}

void shm_vector_swap(shm_vector *vec, shm_vector *with) {
    assert(vec->element_size == with->element_size);
    void *data = vec->data;
    size_t size = vec->size;
    size_t asize = vec->alloc_size;
    vec->data = with->data;
    vec->size = with->size;
    vec->alloc_size = with->alloc_size;
    with->data = data;
    with->size = size;
    with->alloc_size = asize;
}

void *shm_vector_extend(shm_vector *vec) {
    if (vec->size >= vec->alloc_size) {
        // TODO: exp. growth?
        vec->alloc_size += 10;
        assert(0 < vec->element_size);
        vec->data = realloc(vec->data,
                            vec->alloc_size * vec->element_size);
        assert(vec->data != NULL && "Memory re-allocation failed");
    }

    assert(vec->size < vec->alloc_size && "Vector too small");
    assert(0 < vec->element_size);
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

size_t shm_vector_elem_size(shm_vector *vec) {
    assert(0 < vec->element_size);
    return vec->element_size;
}

/*
 * Return the pointer to the element at index 'idx'
 */
void *shm_vector_at(shm_vector *vec, size_t idx) {
        assert(idx < vec->size);
        assert(0 < vec->element_size);
        return (void*)(((unsigned char *)vec->data) + idx*vec->element_size);
}

void *shm_vector_top(shm_vector *vec) {
    if (vec->size == 0)
        return NULL;
    assert(0 < vec->element_size);
    return (void*)(((unsigned char *)vec->data) + (vec->size-1)*vec->element_size);
}
