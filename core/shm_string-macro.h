#ifndef SHAMON_STRING_MACRO_H_
#define SHAMON_STRING_MACRO_H_

#include "vector-macro.h"

#define STRING(name) VEC(name, char)

#define STRING_SIZE(s) VEC_SIZE(s)
#define STRING_ALLOC_SIZE(s) VEC_ALLOC_SIZE(s)

#define STRING_INIT(s) VEC_INIT(s)
#define STRING_DESTROY(s) VEC_DESTROY(s)
#define STRING_AT(s, idx) VEC_AT(s, idx)

#define STRING_GROW(s, new_size) VEC_GROW(s, new_size)

#define STRING_EXTEND(s, outptr)                                                \
    do {                                                                        \
        if (STRING_SIZE(s) >= STRING_ALLOC_SIZE(s)) {                           \
            STRING_ALLOC_SIZE(s)                                                \
                = STRING_ALLOC_SIZE(s) < 32 ? 32 : 2*STRING_ALLOC_SIZE(s);      \
            (s) = realloc((s), STRING_ALLOC_SIZE(s) * sizeof(char));            \
            assert((s) != NULL && "Memory re-allocation failed");               \
        }                                                                       \
        assert((STRING_SIZE(s) < STRING_ALLOC_SIZE(s)) && "Vector too small");  \
        outptr = ((s) + STRING_SIZE(s));                                        \
        STRING_SIZE(s) += 1;                                                    \
    } while (0)

// like EXTEND, just does not change the size
#define STRING_ENSURE_SPACE(s, outptr)                                          \
    do {                                                                        \
        if (STRING_SIZE(s) >= STRING_ALLOC_SIZE(s)) {                           \
            STRING_ALLOC_SIZE(s)                                                \
                = STRING_ALLOC_SIZE(s) < 32 ? 32 : 2*STRING_ALLOC_SIZE(s);      \
            (s) = realloc((s), STRING_ALLOC_SIZE(s) * sizeof(char));            \
            assert((s) != NULL && "Memory re-allocation failed");               \
        }                                                                       \
        assert((STRING_SIZE(s) < STRING_ALLOC_SIZE(s)) && "Vector too small");  \
        outptr = ((s) + STRING_SIZE(s));                                        \
    } while (0)



#define STRING_APPEND(s, c)                                                         \
    do {                                                                            \
        char *__w;                                                                  \
        STRING_EXTEND(s, __w);                                                      \
        assert((STRING_SIZE(s) <= STRING_ALLOC_SIZE(s)) && "String too small");     \
        assert(__w == s + STRING_SIZE(s) - 1);                                      \
        *__w = c;                                                                   \
    } while (0)

#define STRING_POP(s)                                                             \
    do {                                                                          \
        assert(STRING_SIZE(s) > 0);                                               \
        --STRING_SIZE(s);                                                         \
    }   while (0)

#define STRING_TOP_PTR(s) ((s) + STRING_SIZE(s) - 1)
#define STRING_TOP(s) (*STRING_TOP_PTR(s))

#endif /* SHAMON_STRING_MACRO_H_ */
