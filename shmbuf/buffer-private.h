#ifndef SHAMON_SHM_BUFFER_PRIVATE_H
#define SHAMON_SHM_BUFFER_PRIVATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/file.h>

struct source_control;
struct event_record;
struct buffer;

struct buffer *initialize_local_buffer(const char *key,
                                       size_t elem_size,
                                       size_t                 capacity,
                                       struct source_control *control);

void release_local_buffer(struct buffer *buff);

struct buffer *get_shared_buffer(const char *key);
struct buffer *try_get_shared_buffer(const char *key, size_t retry);

#endif /* SHAMON_SHM_BUFFER_PRIVATE_H */
