#ifndef SHAMON_VECTOR_ALIGNED_H_
#define SHAMON_VECTOR_ALIGNED_H_

#include <unistd.h>

#include "vector.h"

/* shm_vector that allocates aligned memory */
typedef struct _shm_vector_aligned {
    shm_vector vec;
    size_t alignment;
} shm_vector_aligned;

void shm_vector_aligned_init(shm_vector_aligned *vec, size_t elem_size,
                             size_t alignment);
size_t shm_vector_aligned_push(shm_vector *vec, void *elem);
void *shm_vector_aligned_extend(shm_vector *vec);
void shm_vector_aligned_swap(shm_vector *vec, shm_vector *with);
void shm_vector_aligned_resize(shm_vector *vec, size_t size);

/* the rest of functions is inherited from shm_vector, use those */

#endif /* SHAMON_VECTOR_ALIGNED_H_ */
