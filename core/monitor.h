/***********************************************
 * Implementation of the buffer for the monitor.
 ************************************************/

#ifndef SHAMON_MONITOR_H_
#define SHAMON_MONITOR_H_

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

typedef struct _shm_monitor_buffer shm_monitor_buffer;
typedef struct _shm_event          shm_event;

void shm_monitor_buffer_init(shm_monitor_buffer *buffer, size_t event_size,
                             size_t capacity);
shm_monitor_buffer *shm_monitor_buffer_create(size_t event_size,
                                              size_t capacity);

void shm_monitor_set_finished(shm_monitor_buffer *buffer);

void shm_monitor_buffer_free(shm_monitor_buffer *buffer);
void shm_monitor_buffer_destroy(shm_monitor_buffer *buffer);

size_t shm_monitor_buffer_elem_size(shm_monitor_buffer *q);
bool   shm_monitor_buffer_active(shm_monitor_buffer *buffer);
size_t shm_monitor_buffer_size(shm_monitor_buffer *buffer);
size_t shm_monitor_buffer_capacity(shm_monitor_buffer *buffer);
size_t shm_monitor_buffer_free_space(shm_monitor_buffer *buffer);
// size_t shm_monitor_buffer_sizeof(void);

/* writer's API */
/* Get write pointer to the buffer, block until space is available */
void *shm_monitor_buffer_write_ptr(shm_monitor_buffer *q);
/* Get write pointer to the buffer or null if no space is available */
void *shm_monitor_buffer_write_ptr_or_null(shm_monitor_buffer *q);
void  shm_monitor_buffer_write_finish(shm_monitor_buffer *q);
/*void shm_monitor_buffer_get_str(shm_monitor_buffer *q, size_t elem); */

/* reader's API */
void *fetch_arbiter_stream(shm_monitor_buffer *buffer);
void  shm_monitor_buffer_consume(shm_monitor_buffer *buffer, size_t n);

/* multiple threads can use top and peek if none of them uses drop/pop
 * at the time
shm_event *shm_monitor_buffer_top(shm_monitor_buffer *buffer);
size_t shm_monitor_buffer_peek(shm_monitor_buffer *buffer, size_t n,
                               void **data1, size_t *size1, void **data2,
                               size_t *size2);
                               */
/* peek 1 event
size_t shm_monitor_buffer_peek1(shm_monitor_buffer *buffer, void **data);
size_t shm_monitor_buffer_drop(shm_monitor_buffer *buffer, size_t n);
bool shm_monitor_buffer_pop(shm_monitor_buffer *q, void *buff);

void *stream_fetch(shm_stream *stream, shm_monitor_buffer *buffer);

void *stream_filter_fetch(shm_stream *stream, shm_monitor_buffer *buffer,
                          shm_stream_filter_fn filter);
                          */
/*
void shm_monitor_buffer_notify_dropped(shm_monitor_buffer *buffer,
                                       uint64_t begin_id, uint64_t end_id);
                                       */

#endif /* SHAMON_MONITOR_H_ */
