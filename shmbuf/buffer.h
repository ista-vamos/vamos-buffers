#ifndef SHAMON_SHM_BUFFER_H
#define SHAMON_SHM_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct source_control;
struct event_record;
struct buffer;

struct buffer *create_shared_buffer(const char *key, size_t elem_size,
                                    const struct source_control *control);
struct buffer *try_get_shared_buffer(const char *key, size_t retry);
struct buffer *get_shared_buffer(const char *key);
struct event_record *buffer_get_avail_events(struct buffer *, size_t *);

void release_shared_buffer(struct buffer *);
void destroy_shared_buffer(struct buffer *);

#if 0
struct buffer *initialize_local_buffer(size_t elem_size);
void free_local_buffer(struct buffer *buff);
/* get a local memory to which we can copy the shared buffer */
//struct buffer *get_local_buffer(struct buffer *shared_buffer);
#endif

bool buffer_is_ready(struct buffer *);
bool buffer_monitor_attached(struct buffer *);
void buffer_set_attached(struct buffer *, bool);

bool buffer_pop(struct buffer *buff, void *dst);
bool buffer_push(struct buffer *buff, const void *elem, size_t size);
void *buffer_get_str(struct buffer *buff, uint64_t elem);

void *buffer_read_pointer(struct buffer *buff, size_t *size);
bool buffer_drop_k(struct buffer *buff, size_t size);
size_t buffer_consume(struct buffer *buff, size_t k);

size_t buffer_size(struct buffer *buff);
size_t buffer_capacity(struct buffer *buff);
size_t buffer_elem_size(struct buffer *buff);

void *buffer_start_push(struct buffer *buff);

void *buffer_partial_push(struct buffer *buff, void *prev_push,
                          const void *elem, size_t size);
/* evid may be ~(0LL), but then there's no garbage collection */
void *buffer_partial_push_str(struct buffer *buff, void *prev_push,
                              uint64_t evid, const char *str);
void *buffer_partial_push_str_n(struct buffer *buff, void *prev_push,
                                uint64_t evid, const char *str, size_t len);
void buffer_finish_push(struct buffer *buff);

struct aux_buff_ptr {
    uint32_t buffer_id;
    uint32_t offset;
} __attribute__((packed, aligned(64)));

void buffer_set_last_processed_id(struct buffer *buff, uint64_t id);
void buffer_notify_dropped(struct buffer *buffer, uint64_t begin_id,
                           uint64_t end_id);

#endif /* SHAMON_SHM_BUFFER_H */
