#ifndef SHAMON_SHM_AUX_BUFFER_H
#define SHAMON_SHM_AUX_BUFFER_H

#include <stddef.h>
#include <stdbool.h>

struct aux_buffer;

struct aux_buffer *get_shared_aux_buffer(const char *key);
struct aux_buffer *initialize_shared_aux_buffer(size_t elem_size);

void release_shared_aux_buffer(struct aux_buffer *);
void destroy_shared_aux_buffer(struct aux_buffer *);
/* get a local memory to which we can copy the shared buffer */
//struct aux_buffer *get_local_buffer(struct aux_buffer *shared_buffer);

bool buffer_is_ready(struct aux_buffer *);
bool buffer_monitor_attached(struct aux_buffer *);
void buffer_set_attached(struct aux_buffer *, bool);

bool aux_buffer_pop(struct aux_buffer *buff, void *dst);
bool aux_buffer_push(struct aux_buffer *buff, const void *elem, size_t size);
void *aux_buffer_read_pointer(struct aux_buffer *buff, size_t *size);
size_t aux_buffer_size(struct aux_buffer *buff);
size_t aux_buffer_capacity(struct aux_buffer *buff);
size_t aux_buffer_elem_size(struct aux_buffer *buff);

void *aux_buffer_start_push(struct aux_buffer *buff);
void *aux_buffer_partial_push(struct aux_buffer *buff, void *prev_push,
                          const void *elem, size_t size);
bool aux_buffer_finish_push(struct aux_buffer *buff);

#endif /* SHAMON_SHM_AUX_BUFFER_H */
