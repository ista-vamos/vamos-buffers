#include "vamos-buffers/core/vector.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void vms_vector_init(vms_vector *vec, size_t elem_size) {
    assert(elem_size > 0);
    memset(vec, 0, sizeof(*vec));
    vec->element_size = elem_size;
}

void vms_vector_destroy(vms_vector *vec) { free(vec->data); }

void vms_vector_swap(vms_vector *vec, vms_vector *with) {
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

void vms_vector_resize(vms_vector *vec, size_t size) {
    if (vec->size >= size) {
        return;
    }

    if (size >= vec->alloc_size) {
        // TODO: exp. growth?
        vec->alloc_size = size;
        assert(0 < vec->element_size);
        vec->data = realloc(vec->data, vec->alloc_size * vec->element_size);
        if (vec->data == NULL) {
            assert(0 && "Memory re-allocation failed");
            abort();
        }
    }

    void *addr = ((unsigned char *)vec->data) + vec->size * vec->element_size;
    memset(addr, 0, size - vec->size);
    vec->size = size;
}

void *vms_vector_extend(vms_vector *vec) {
    if (vec->size >= vec->alloc_size) {
        // TODO: exp. growth?
        vec->alloc_size += 10;
        assert(0 < vec->element_size);
        vec->data = realloc(vec->data, vec->alloc_size * vec->element_size);
        assert(vec->data != NULL && "Memory re-allocation failed");
    }

    assert(vec->size < vec->alloc_size && "Vector too small");
    assert(0 < vec->element_size);
    void *addr = ((unsigned char *)vec->data) + vec->size * vec->element_size;
    ++vec->size;
    return addr;
}

size_t vms_vector_push(vms_vector *vec, void *elem) {
    memcpy(vms_vector_extend(vec), elem, vec->element_size);
    return vec->size;
}

size_t vms_vector_pop(vms_vector *vec) {
    assert(vec->size > 0);
    return --vec->size;
}

size_t vms_vector_size(vms_vector *vec) { return vec->size; }

size_t vms_vector_elem_size(vms_vector *vec) {
    assert(0 < vec->element_size);
    return vec->element_size;
}

void *vms_vector_at(vms_vector *vec, size_t idx) {
    assert(idx < vec->size);
    assert(0 < vec->element_size);
    return (void *)(((unsigned char *)vec->data) + idx * vec->element_size);
}

void *vms_vector_at_checked(vms_vector *vec, size_t idx) {
    assert(0 < vec->element_size);

    if (idx < vec->size) {
        return (void *)(((unsigned char *)vec->data) + idx * vec->element_size);
    }

    return NULL;
}

void *vms_vector_top(vms_vector *vec) {
    if (vec->size == 0)
        return NULL;
    assert(0 < vec->element_size);
    return (void *)(((unsigned char *)vec->data) +
                    (vec->size - 1) * vec->element_size);
}
