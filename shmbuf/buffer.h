#ifndef SHAMON_SHM_BUFFER_H
#define SHAMON_SHM_BUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct source_control;
struct buffer;

struct buffer *get_shared_buffer(const char *key);
struct buffer *initialize_shared_buffer(const char *key,
                                        size_t elem_size,
                                        struct source_control *control);
void *initialize_shared_control_buffer(const char *buff_key, size_t size);
void *get_shared_control_buffer(const char *buff_key);

void release_shared_control_buffer(const char *buffkey, void *buffer);
void release_shared_buffer(struct buffer *);
void destroy_shared_buffer(struct buffer *);

struct buffer *initialize_local_buffer(size_t elem_size);
void free_local_buffer(struct buffer *buff);
/* get a local memory to which we can copy the shared buffer */
//struct buffer *get_local_buffer(struct buffer *shared_buffer);

bool buffer_is_ready(struct buffer *);
bool buffer_monitor_attached(struct buffer *);
void buffer_set_attached(struct buffer *, bool);

bool buffer_pop(struct buffer *buff, void *dst);
bool buffer_pop_k(struct buffer *buff, void *dst, size_t k);
bool buffer_push(struct buffer *buff, const void *elem, size_t size);
void *buffer_get_str(struct buffer *buff, uint64_t elem);

void *buffer_read_pointer(struct buffer *buff, size_t *size);
bool buffer_drop_k(struct buffer *buff, size_t size);

size_t buffer_size(struct buffer *buff);
size_t buffer_capacity(struct buffer *buff);
size_t buffer_elem_size(struct buffer *buff);

size_t control_buffer_size(void *buff);

void *buffer_start_push(struct buffer *buff);

void *buffer_partial_push(struct buffer *buff, void *prev_push,
                          const void *elem, size_t size);
void *buffer_partial_push_str(struct buffer *buff, void *prev_push,
                              const char *str);
void *buffer_partial_push_str_n(struct buffer *buff, void *prev_push,
                                const char *str, size_t len);
bool buffer_finish_push(struct buffer *buff);

struct aux_buff_ptr {
    uint32_t buffer_id;
    uint32_t offset;
} __attribute__((packed,aligned(64)));

#endif /* SHAMON_SHM_BUFFER_H */
