/***********************************************
 * Implementation of the buffer for the arbiter.
 *
 * It is a ring buffer that supports one reader and one writer
 * that work in parallel. The reader and writer API cannot be mixed
 * in one thread/process (unless using some additional locking).
 * That is, using writer (reader) API by more than two threads
 * at the same time is not thread-safe.
 ************************************************/

#ifndef VAMOS_ARBITER_H_
#define VAMOS_ARBITER_H_

#include <stdbool.h>
#include <unistd.h>

#include "stream.h"

typedef struct _vms_arbiter_buffer vms_arbiter_buffer;
typedef struct _vms_event vms_event;

void vms_arbiter_buffer_init(vms_arbiter_buffer *buffer, vms_stream *stream,
                             size_t out_event_size, size_t capacity);
vms_arbiter_buffer *vms_arbiter_buffer_create(vms_stream *stream,
                                              size_t out_event_size,
                                              size_t capacity);
size_t vms_arbiter_buffer_set_drop_space_threshold(vms_arbiter_buffer *buffer,
                                                   size_t thr);

void vms_arbiter_buffer_free(vms_arbiter_buffer *buffer);
void vms_arbiter_buffer_destroy(vms_arbiter_buffer *buffer);
void vms_arbiter_buffer_set_active(vms_arbiter_buffer *buffer, bool val);
size_t vms_arbiter_buffer_elem_size(vms_arbiter_buffer *q);
vms_stream *vms_arbiter_buffer_stream(vms_arbiter_buffer *buffer);
bool vms_arbiter_buffer_active(vms_arbiter_buffer *buffer);
size_t vms_arbiter_buffer_size(vms_arbiter_buffer *buffer);
size_t vms_arbiter_buffer_capacity(vms_arbiter_buffer *buffer);
size_t vms_arbiter_buffer_free_space(vms_arbiter_buffer *buffer);
size_t vms_arbiter_buffer_sizeof(void);

/* writer's API */
void vms_arbiter_buffer_push(vms_arbiter_buffer *q, const void *elem,
                             size_t size);

/* cannot be mixed with push */

void *vms_arbiter_buffer_write_ptr(vms_arbiter_buffer *q);
void vms_arbiter_buffer_write_finish(vms_arbiter_buffer *q);
void vms_arbiter_buffer_get_str(vms_arbiter_buffer *q, size_t elem);

/* reader's API */
/* multiple threads can use top and peek if none of them uses drop/pop
 * at the time */
vms_event *vms_arbiter_buffer_top(vms_arbiter_buffer *buffer);
size_t vms_arbiter_buffer_peek(vms_arbiter_buffer *buffer, size_t n,
                               void **data1, size_t *size1, void **data2,
                               size_t *size2);
/* peek 1 event */
size_t vms_arbiter_buffer_peek1(vms_arbiter_buffer *buffer, void **data);
size_t vms_arbiter_buffer_drop(vms_arbiter_buffer *buffer, size_t n);
size_t vms_arbiter_buffer_drop_older_than(vms_arbiter_buffer *buffer,
                                          vms_eventid id);
bool vms_arbiter_buffer_pop(vms_arbiter_buffer *q, void *buff);

void *stream_fetch(vms_stream *stream, vms_arbiter_buffer *buffer);

void *stream_filter_fetch(vms_stream *stream, vms_arbiter_buffer *buffer,
                          vms_stream_filter_fn filter);

bool vms_arbiter_buffer_is_done(vms_arbiter_buffer *buffer);

size_t vms_arbiter_buffer_dropped_num(vms_arbiter_buffer *buffer);
size_t vms_arbiter_buffer_dropped_times(vms_arbiter_buffer *buffer);
size_t vms_arbiter_buffer_written_num(vms_arbiter_buffer *buffer);

void vms_arbiter_buffer_notify_dropped(vms_arbiter_buffer *buffer,
                                       uint64_t begin_id, uint64_t end_id);

void vms_arbiter_buffer_dump_stats(vms_arbiter_buffer *buffer);
#endif /* VAMOS_ARBITER_H_ */
