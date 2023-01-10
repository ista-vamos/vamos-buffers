#ifndef SHAMON_VECTOR_H_
#define SHAMON_VECTOR_H_

/**
 * Implementation of a vector similar to std::vector in C++.
 *
 * For inlined vector implementation (resizible memory array),
 * see `vector-macro.h`.
 */

#include <unistd.h>

typedef struct _shm_vector {
    size_t size;
    size_t element_size;
    size_t alloc_size;
    void *data;
} shm_vector;

void shm_vector_init(shm_vector *vec, size_t elem_size);
void shm_vector_destroy(shm_vector *vec);

size_t shm_vector_size(shm_vector *vec);
size_t shm_vector_elem_size(shm_vector *vec);


/**
 * Return the pointer to the element at index `idx`.
 *
 * In debug mode, OOB checks are performed.
 * In release mode, if `idx` is OOB, the behavior is undefined.
 */
void *shm_vector_at(shm_vector *vec, size_t idx);

/**
 * Return the pointer to the element at index `idx`.
 * If `idx` is OOB, return NULL.
 */
void *shm_vector_at_checked(shm_vector *vec, size_t idx);


/**
 * Push an element pointed by `elem` into `vec`.
 * Memory is reallocated if needed. By default, the new memory can store
 * 10 more elements than the previously allocated memory.
 */
size_t shm_vector_push(shm_vector *vec, void *elem);


/**
 * Return the pointer to the next free space in the vector and increase
 * the number of elements in the vector by one.
 * The returned pointer should be used to write the element into the vector,
 * e.g.:
 *
 *   int *ptr = shm_vector_extend(vec);
 *   *ptr = 1;
 *
 * If there is not enough space to write a single element, the memory is
 * reallocated, by default to hold 10 new elements.
 */
void *shm_vector_extend(shm_vector *vec);

/**
 * Remove the last element from the vector (decrease its size by 1)
 */
size_t shm_vector_pop(shm_vector *vec);


/**
 * Return the pointer to the top element or NULL
 * if the vector is empty.
 */
void *shm_vector_top(shm_vector *vec);


/**
 * Swap the contents of two vectors.
 */
void shm_vector_swap(shm_vector *vec, shm_vector *with);


/**
 * Resize the vector to hold `size` elements. If the current size
 * of the vector is smaller than `size`, do nothing.
 *
 * This function zeroes the memory at indices [vec->size, size - 1],
 * i.e., all elements on indices [0, size) are either written or 0'ed.
 *
 * Note that the vector may have allocated memory for more than `size`
 * elements. In that case the memory remains non-0'ed.
 * This should not be a problem as the uninitialized memory should not be touched
 * as the user knows only that there is space for `size` elements
 * but nothing more.
 */
void shm_vector_resize(shm_vector *vec, size_t size);

#endif /* SHAMON_VECTOR_H_ */
