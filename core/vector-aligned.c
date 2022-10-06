#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "vector-aligned.h"

void shm_vector_aligned_init(shm_vector_aligned *vec, size_t elem_size, size_t alignment) {
    assert(alignment > 0);
    shm_vector_init(&vec->vec, elem_size);
    vec->alignment = alignment;
}

void shm_vector_aligned_swap(shm_vector *vec, shm_vector *with) {
    shm_vector_swap(vec, with);

    shm_vector_aligned *avec = (shm_vector_aligned*) vec;
    size_t alignment =  avec->alignment;
    avec->alignment  =  ((shm_vector_aligned *)with)->alignment;
    ((shm_vector_aligned *)with)->alignment = alignment;
}

void shm_vector_aligned_resize(shm_vector *vec, size_t size) {
    shm_vector_aligned *avec = (shm_vector_aligned*) vec;

    if (vec->size >= size)
        return;
    if (size >= vec->alloc_size) {
        // TODO: exp. growth?
        vec->alloc_size = size;
        assert(0 < vec->element_size);
        void *new_data = xalloc_aligned(vec->alloc_size * vec->element_size,
                                        avec->alignment);
        if (vec->size > 0)
                memcpy(new_data, vec->data, vec->size);
        vec->data = new_data;
    }

    void *addr = ((unsigned char *)vec->data) + vec->size * vec->element_size;
    memset(addr, 0, size - vec->size);
    vec->size = size;
}

void *shm_vector_aligned_extend(shm_vector *vec) {
    shm_vector_aligned *avec = (shm_vector_aligned*) vec;

    if (vec->size >= vec->alloc_size) {
        // TODO: exp. growth?
        vec->alloc_size += 10;
        assert(0 < vec->element_size);
        void *new_data = xalloc_aligned(vec->alloc_size * vec->element_size,
                                        avec->alignment);
        if (vec->size > 0)
                memcpy(new_data, vec->data, vec->size);
        vec->data = new_data;
    }

    assert(vec->size < vec->alloc_size && "Vector too small");
    assert(0 < vec->element_size);
    void *addr = ((unsigned char *)vec->data) + vec->size * vec->element_size;
    ++vec->size;
    return addr;
}

size_t shm_vector_aligned_push(shm_vector *vec, void *elem) {
    memcpy(shm_vector_aligned_extend(vec), elem, vec->element_size);
    return vec->size;
}
