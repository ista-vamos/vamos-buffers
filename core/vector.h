#ifndef SHAMON_VECTOR_H_
#define SHAMON_VECTOR_H_

#include <unistd.h>

typedef struct _shm_vector {
    size_t size;
    size_t element_size;
    size_t alloc_size;
    void *data;
} shm_vector;

void shm_vector_init(shm_vector *vec, size_t elem_size);
void shm_vector_destroy(shm_vector *vec);
size_t shm_vector_push(shm_vector *vec, void *elem);
void *shm_vector_extend(shm_vector *vec);
size_t shm_vector_pop(shm_vector *vec);
size_t shm_vector_size(shm_vector *vec);
size_t shm_vector_elem_size(shm_vector *vec);
void *shm_vector_at(shm_vector *vec, size_t idx);
void *shm_vector_top(shm_vector *vec);
void shm_vector_swap(shm_vector *vec, shm_vector *with);
void shm_vector_resize(shm_vector *vec, size_t size);

#endif /* SHAMON_VECTOR_H_ */
