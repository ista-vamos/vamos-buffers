#include "vamos-buffers/core/vector-aligned.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "vamos-buffers/core/utils.h"

void vms_vector_aligned_init(vms_vector_aligned *vec, size_t elem_size,
                             size_t alignment) {
    assert(alignment > 0);
    vms_vector_init(&vec->vec, elem_size);
    vec->alignment = alignment;
}

void vms_vector_aligned_swap(vms_vector *vec, vms_vector *with) {
    vms_vector_swap(vec, with);

    vms_vector_aligned *avec = (vms_vector_aligned *)vec;
    size_t alignment = avec->alignment;
    avec->alignment = ((vms_vector_aligned *)with)->alignment;
    ((vms_vector_aligned *)with)->alignment = alignment;
}

void vms_vector_aligned_resize(vms_vector *vec, size_t size) {
    vms_vector_aligned *avec = (vms_vector_aligned *)vec;

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

void *vms_vector_aligned_extend(vms_vector *vec) {
    vms_vector_aligned *avec = (vms_vector_aligned *)vec;

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

size_t vms_vector_aligned_push(vms_vector *vec, void *elem) {
    memcpy(vms_vector_aligned_extend(vec), elem, vec->element_size);
    return vec->size;
}
