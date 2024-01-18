#ifndef VAMOS_VECTOR_MACRO_H_
#define VAMOS_VECTOR_MACRO_H_

/**
 * Macro for creating inlined vector, that is, a piece of continuous memory
 * that is resized when it is filled. It just wraps up the classical pattern:
 *
 *   // allocate memory
 *   ptr = malloc(alloc_size)
 *
 *   // ... fill memory
 *
 *   // ... resize memory if full
 *   if (size >= alloc_size) {
 *       ptr = realloc(ptr, ...)
 *   }
 *
 * The basic macro is VEC(name, elem_ty) that declares three variables:
 *
 *   elem_ty *name
 *   size_t name_size;
 *   size_t name_alloc_size;
 *
 *  These variables could be used manually, e.g.:
 *
 *    name_size = 0;
 *    name = malloc(4*sizeof(int));
 *    name_alloc_size = 4;
 *
 *    for (int i = 0; i < 10; ++i) {
 *      if (name_size >= name_alloc_size) {
 *        name_alloc_size += 10;
 *        name = realloc(name, name_alloc_size);
 *      }
 *      name[i] = i;
 *      ++name_size;
 *    }
 *
 *  However, this file defines macros to wrap all the work:
 *
 *  VEC(v, int);
 *  VEC_INIT(v);
 *  for (int i = 0; i < 10; ++i) {
 *    VEC_PUSH(v, &i);
 *  }
 *
 *  VEC() can be usually also used in struct declarations, but not always.
 *  For example if a struct is opaque and the vector must be accessed from
 *  outside. In such cases, we provide `vms_vector` data type.
 */

#include <string.h>

#define VEC(name, elem_ty) \
    elem_ty *name;         \
    size_t name##_size;    \
    size_t name##_alloc_size

#define VEC_SIZE(vec) vec##_size
#define VEC_ALLOC_SIZE(vec) vec##_alloc_size
#define VEC_ELEM_SIZE(vec) (sizeof(*(vec)))

#define VEC_INIT(vec)            \
    do {                         \
        (vec) = NULL;            \
        VEC_SIZE(vec) = 0;       \
        VEC_ALLOC_SIZE(vec) = 0; \
    } while (0)

#define VEC_DESTROY(vec) \
    do {                 \
        free((vec));     \
    } while (0)

/**
 * Return pointer to the `idx`th element of the vector.
 * For a vector VEC(vec, elem_ty) it is the same as
 * manually doing
 *
 *   vec[idx]
 *
 * Accessing the vector manually is fine, too.
 */
#define VEC_AT(vec, idx) (*((vec) + idx))

/**
 * Grow the vector to hold `new_size` elements. If `new_size` is less or equal
 * to the current allocated capacity of the vector, do nothing.
 */
#define VEC_GROW(vec, new_size)                                               \
    do {                                                                      \
        if (new_size > VEC_ALLOC_SIZE(vec)) {                                 \
            VEC_ALLOC_SIZE(vec) = (new_size);                                 \
            (vec) = realloc((vec), VEC_ALLOC_SIZE(vec) * VEC_ELEM_SIZE(vec)); \
            assert((vec) != NULL && "Memory re-allocation failed");           \
        }                                                                     \
    } while (0)

/**
 * Return the pointer to the next free space in the vector and increase
 * the number of elements in the vector by one.
 * The returned pointer should be used to write the element into the vector,
 * e.g.:
 *
 *   int *ptr = VEC_EXTEND_N(vec, 4);
 *   *ptr = 1;
 *
 * If there is not enough space to write a single element, the memory is
 * reallocated to hold `n` new elements.
 */
#define VEC_EXTEND_N(vec, outptr, n)                                          \
    do {                                                                      \
        if (VEC_SIZE(vec) >= VEC_ALLOC_SIZE(vec)) {                           \
            VEC_ALLOC_SIZE(vec) += (n);                                       \
            (vec) = realloc((vec), VEC_ALLOC_SIZE(vec) * VEC_ELEM_SIZE(vec)); \
            assert((vec) != NULL && "Memory re-allocation failed");           \
        }                                                                     \
        assert((VEC_SIZE(vec) < VEC_ALLOC_SIZE(vec)) && "Vector too small");  \
        outptr =                                                              \
            ((unsigned char *)(vec)) + VEC_SIZE(vec) * VEC_ELEM_SIZE(vec);    \
        VEC_SIZE(vec) += 1;                                                   \
    } while (0)

#define VEC_EXTEND(vec, outptr) VEC_EXTEND_N(vec, outptr, 16)

/**
 * Push an element pointed by `elemptr` into `vec`.
 * Memory is reallocated if needed. By default, the new memory can store
 * 10 more elements than the previously allocated memory.
 */
#define VEC_PUSH(vec, elemptr)                                                \
    do {                                                                      \
        if (VEC_SIZE(vec) >= VEC_ALLOC_SIZE(vec)) {                           \
            VEC_ALLOC_SIZE(vec) += 10;                                        \
            (vec) = realloc((vec), VEC_ALLOC_SIZE(vec) * VEC_ELEM_SIZE(vec)); \
            assert((vec) != NULL && "Memory re-allocation failed");           \
        }                                                                     \
        assert((VEC_SIZE(vec) < VEC_ALLOC_SIZE(vec)) && "Vector too small");  \
        memcpy(((unsigned char *)(vec)) + VEC_SIZE(vec) * VEC_ELEM_SIZE(vec), \
               elemptr, VEC_ELEM_SIZE(vec));                                  \
        VEC_SIZE(vec) += 1;                                                   \
    } while (0)

/**
 * Remove the last element from the vector (decrease its size by 1)
 */
#define VEC_POP(vec)     \
    do {                 \
        --VEC_SIZE(vec); \
    } while (0)

/**
 * Remove the first occurence of  `elem` from `vec`. `elem` must be a simple
 * type, since we use == with it.
 * IMPORTANT: The removal does shuffle the elements, in particular,
 * the last element will be copied to the place of the removed one.
 */
#define VEC_UNORD_REMOVE(vec, elem)               \
    do {                                          \
        for (int i = 0; i < VEC_SIZE(vec); ++i) { \
            if (VEC_AT(i) == elem) {              \
                vec[i] = vec[VEC_SIZE(vec) - 1];  \
                --VEC_SIZE(vec);                  \
                break;                            \
            }                                     \
        }                                         \
        while (0)

/**
 * Return the pointer to the top element in the vector.
 * There is no check if there is any element. If there is none,
 * the pointer underflows and causes UB.
 */
#define VEC_TOP_PTR(vec) ((vec) + VEC_SIZE(vec) - 1)

/**
 * Return the top element in the vector. The pointer to the element
 * must be dereferencable, e.g., the contents of the vector cannot
 * be an opaque structure with a known size (which could be theoretically
 * stored in the vector as it pushes elements using memcpy()).
 *
 * There is no check if there is any element. If there is none,
 * the pointer underflows and causes UB.
 */
#define VEC_TOP(vec) (*VEC_TOP_PTR(vec))

/**
 * Pop and return the top element of the vector.
 *
 * There is no check if there is any element. If there is none,
 * the pointer underflows and causes UB.
 */
#define VEC_POP_TOP(vec) (--VEC_SIZE(vec), *((vec) + VEC_SIZE(vec)))

#endif /* VAMOS_VECTOR_MACRO_H_ */
