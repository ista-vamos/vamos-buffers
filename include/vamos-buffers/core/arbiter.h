/***********************************************
 * Implementation of the buffer for the arbiter.
 *
 * It is a ring buffer that supports one reader and one writer
 * that work in parallel. The reader and writer API cannot be mixed
 * in one thread/process (unless using some additional locking).
 * That is, using writer (reader) API by more than two threads
 * at the same time is not thread-safe.
 ************************************************/

#ifndef SHAMON_ARBITER_H_
#define SHAMON_ARBITER_H_

#include <stdbool.h>
#include <unistd.h>

#include "stream.h"

typedef struct _shm_arbiter_buffer shm_arbiter_buffer;
typedef struct _shm_event shm_event;

void shm_arbiter_buffer_init(shm_arbiter_buffer *buffer, shm_stream *stream,
                             size_t out_event_size, size_t capacity);
shm_arbiter_buffer *shm_arbiter_buffer_create(shm_stream *stream,
                                              size_t out_event_size,
                                              size_t capacity);
size_t shm_arbiter_buffer_set_drop_space_threshold(shm_arbiter_buffer *buffer,
                                                   size_t thr);

void shm_arbiter_buffer_free(shm_arbiter_buffer *buffer);
void shm_arbiter_buffer_destroy(shm_arbiter_buffer *buffer);
void shm_arbiter_buffer_set_active(shm_arbiter_buffer *buffer, bool val);
size_t shm_arbiter_buffer_elem_size(shm_arbiter_buffer *q);
shm_stream *shm_arbiter_buffer_stream(shm_arbiter_buffer *buffer);
bool shm_arbiter_buffer_active(shm_arbiter_buffer *buffer);
size_t shm_arbiter_buffer_size(shm_arbiter_buffer *buffer);
size_t shm_arbiter_buffer_capacity(shm_arbiter_buffer *buffer);
size_t shm_arbiter_buffer_free_space(shm_arbiter_buffer *buffer);
size_t shm_arbiter_buffer_sizeof(void);

/* writer's API */
void shm_arbiter_buffer_push(shm_arbiter_buffer *q, const void *elem,
                             size_t size);

/* cannot be mixed with push */

void *shm_arbiter_buffer_write_ptr(shm_arbiter_buffer *q);
void shm_arbiter_buffer_write_finish(shm_arbiter_buffer *q);
void shm_arbiter_buffer_get_str(shm_arbiter_buffer *q, size_t elem);

/* reader's API */
/* multiple threads can use top and peek if none of them uses drop/pop
 * at the time */
shm_event *shm_arbiter_buffer_top(shm_arbiter_buffer *buffer);
size_t shm_arbiter_buffer_peek(shm_arbiter_buffer *buffer, size_t n,
                               void **data1, size_t *size1, void **data2,
                               size_t *size2);
/* peek 1 event */
size_t shm_arbiter_buffer_peek1(shm_arbiter_buffer *buffer, void **data);
size_t shm_arbiter_buffer_drop(shm_arbiter_buffer *buffer, size_t n);
size_t shm_arbiter_buffer_drop_older_than(shm_arbiter_buffer *buffer,
                                          shm_eventid id);
bool shm_arbiter_buffer_pop(shm_arbiter_buffer *q, void *buff);

void *stream_fetch(shm_stream *stream, shm_arbiter_buffer *buffer);

void *stream_filter_fetch(shm_stream *stream, shm_arbiter_buffer *buffer,
                          shm_stream_filter_fn filter);

bool shm_arbiter_buffer_is_done(shm_arbiter_buffer *buffer);

size_t shm_arbiter_buffer_dropped_num(shm_arbiter_buffer *buffer);
size_t shm_arbiter_buffer_dropped_times(shm_arbiter_buffer *buffer);
size_t shm_arbiter_buffer_written_num(shm_arbiter_buffer *buffer);

void shm_arbiter_buffer_notify_dropped(shm_arbiter_buffer *buffer,
                                       uint64_t begin_id, uint64_t end_id);

void shm_arbiter_buffer_dump_stats(shm_arbiter_buffer *buffer);
#endif /* SHAMON_ARBITER_H_ */
