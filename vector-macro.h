#ifndef SHAMON_VECTOR_MACRO_H_
#define SHAMON_VECTOR_MACRO_H_

#define VEC(name, elem_ty)\
    elem_ty *name; size_t name##_size; size_t name##_alloc_size

#define VEC_SIZE(vec) vec##_size
#define VEC_ALLOC_SIZE(vec) vec##_alloc_size
#define VEC_ELEM_SIZE(vec)  (sizeof(*(vec)))

#define VEC_INIT(vec) do {   \
    (vec) = NULL;            \
    VEC_SIZE(vec) = 0;       \
    VEC_ALLOC_SIZE(vec) = 0; \
    } while(0)

#define VEC_AT(vec, idx) (*((vec)+idx))

#define VEC_EXTEND(vec, outptr) do {                                      \
    if (VEC_SIZE(vec) >= VEC_ALLOC_SIZE(vec)) {                           \
        VEC_ALLOC_SIZE(vec) += 10;                                        \
        (vec) = realloc((vec), VEC_ALLOC_SIZE(vec)*VEC_ELEM_SIZE(vec));   \
        assert((vec) != NULL && "Memory re-allocation failed");           \
    }                                                                     \
    assert((VEC_SIZE(vec) < VEC_ALLOC_SIZE(vec)) && "Vector too small");  \
    outptr = ((unsigned char *)(vec)) + VEC_SIZE(vec)*VEC_ELEM_SIZE(vec); \
    VEC_SIZE(vec) += 1;                                                   \
    } while(0)

#define VEC_PUSH(vec, elemptr) do {                                       \
    if (VEC_SIZE(vec) >= VEC_ALLOC_SIZE(vec)) {                           \
        VEC_ALLOC_SIZE(vec) += 10;                                        \
        (vec) = realloc((vec), VEC_ALLOC_SIZE(vec)*VEC_ELEM_SIZE(vec));   \
        assert((vec) != NULL && "Memory re-allocation failed");           \
    }                                                                     \
    assert((VEC_SIZE(vec) < VEC_ALLOC_SIZE(vec)) && "Vector too small");  \
    memcpy(((unsigned char *)(vec)) + VEC_SIZE(vec)*VEC_ELEM_SIZE(vec),   \
           elemptr, VEC_ELEM_SIZE(vec));                                  \
    VEC_SIZE(vec) += 1;                                                   \
    } while(0)

#define VEC_POP(vec) do {  \
    -- VEC_SIZE(vec);      \
    while(0)

#define VEC_TOP_PTR(vec) ((vec) + VEC_SIZE(vec) - 1)
#define VEC_TOP(vec) (*VEC_TOP_PTR(vec))

#endif /* SHAMON_VECTOR_H_ */
