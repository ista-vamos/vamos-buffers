/***********************************************
 * Implementation of the buffer for the monitor.
 ************************************************/

#ifndef SHAMON_MONITOR_H_
#define SHAMON_MONITOR_H_

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

typedef struct _vms_monitor_buffer vms_monitor_buffer;
typedef struct _vms_event vms_event;

void vms_monitor_buffer_init(vms_monitor_buffer *buffer, size_t event_size,
                             size_t capacity);
vms_monitor_buffer *vms_monitor_buffer_create(size_t event_size,
                                              size_t capacity);

void vms_monitor_set_finished(vms_monitor_buffer *buffer);

void vms_monitor_buffer_free(vms_monitor_buffer *buffer);
void vms_monitor_buffer_destroy(vms_monitor_buffer *buffer);

size_t vms_monitor_buffer_elem_size(vms_monitor_buffer *q);
bool vms_monitor_buffer_active(vms_monitor_buffer *buffer);
size_t vms_monitor_buffer_size(vms_monitor_buffer *buffer);
size_t vms_monitor_buffer_capacity(vms_monitor_buffer *buffer);
size_t vms_monitor_buffer_free_space(vms_monitor_buffer *buffer);
// size_t vms_monitor_buffer_sizeof(void);

/* writer's API */
/* Get write pointer to the buffer, block until space is available */
void *vms_monitor_buffer_write_ptr(vms_monitor_buffer *q);
/* Get write pointer to the buffer or null if no space is available */
void *vms_monitor_buffer_write_ptr_or_null(vms_monitor_buffer *q);
void vms_monitor_buffer_write_finish(vms_monitor_buffer *q);
/*void vms_monitor_buffer_get_str(vms_monitor_buffer *q, size_t elem); */

/* reader's API */
void *fetch_arbiter_stream(vms_monitor_buffer *buffer);
void vms_monitor_buffer_consume(vms_monitor_buffer *buffer, size_t n);

/* multiple threads can use top and peek if none of them uses drop/pop
 * at the time
vms_event *vms_monitor_buffer_top(vms_monitor_buffer *buffer);
size_t vms_monitor_buffer_peek(vms_monitor_buffer *buffer, size_t n,
                               void **data1, size_t *size1, void **data2,
                               size_t *size2);
                               */
/* peek 1 event
size_t vms_monitor_buffer_peek1(vms_monitor_buffer *buffer, void **data);
size_t vms_monitor_buffer_drop(vms_monitor_buffer *buffer, size_t n);
bool vms_monitor_buffer_pop(vms_monitor_buffer *q, void *buff);

void *stream_fetch(vms_stream *stream, vms_monitor_buffer *buffer);

void *stream_filter_fetch(vms_stream *stream, vms_monitor_buffer *buffer,
                          vms_stream_filter_fn filter);
                          */
/*
void vms_monitor_buffer_notify_dropped(vms_monitor_buffer *buffer,
                                       uint64_t begin_id, uint64_t end_id);
                                       */

#endif /* SHAMON_MONITOR_H_ */
