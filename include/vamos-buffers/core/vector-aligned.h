#ifndef VAMOS_VECTOR_ALIGNED_H_
#define VAMOS_VECTOR_ALIGNED_H_

#include <unistd.h>

#include "vector.h"

/* vms_vector that allocates aligned memory */
typedef struct _vms_vector_aligned {
    vms_vector vec;
    size_t alignment;
} vms_vector_aligned;

void vms_vector_aligned_init(vms_vector_aligned *vec, size_t elem_size,
                             size_t alignment);
size_t vms_vector_aligned_push(vms_vector *vec, void *elem);
void *vms_vector_aligned_extend(vms_vector *vec);
void vms_vector_aligned_swap(vms_vector *vec, vms_vector *with);
void vms_vector_aligned_resize(vms_vector *vec, size_t size);

/* the rest of functions is inherited from vms_vector, use those */

#endif /* VAMOS_VECTOR_ALIGNED_H_ */
