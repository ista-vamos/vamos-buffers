#ifndef SHAMON_STREAMS_H
#define SHAMON_STREAMS_H

#include "event.h"

typedef struct _shm_arbiter_buffer shm_arbiter_buffer;

void shm_arbiter_buffer_push(shm_arbiter_buffer *q, const void *elem, size_t size);
void *shm_arbiter_buffer_start_push(shm_arbiter_buffer *q, size_t *size);
void shm_arbiter_buffer_push_k(shm_arbiter_buffer *q, const void *elems, size_t size);
void shm_arbiter_buffer_finish_push(shm_arbiter_buffer *q);
/*
bool shm_arbiter_buffer_pop(shm_arbiter_buffer *q, void *buff);
size_t shm_arbiter_buffer_pop_k(shm_arbiter_buffer *q, void *buff);
size_t shm_arbiter_buffer_size(shm_arbiter_buffer *q);
size_t shm_arbiter_buffer_elem_size(shm_arbiter_buffer *q);
size_t shm_arbiter_buffer_capacity(shm_arbiter_buffer *q);
*/

typedef bool (*shm_stream_has_event_fn)(struct _shm_stream *);
//typedef shm_event *(*shm_stream_get_next_event_fn)(struct _shm_stream *);
typedef size_t (*shm_stream_buffer_events_fn)(struct _shm_stream *,
                                              shm_arbiter_buffer *buffer);

// TODO: make this opaque
typedef struct _shm_stream {
    uint64_t id;
    const char *name;
    shm_eventid last_event_id;
    size_t event_size;
    /* callbacks */
    shm_stream_has_event_fn has_event;
    shm_stream_buffer_events_fn buffer_events;
    /* shm_stream_get_next_event_fn get_next_event; */
} shm_stream;

void shm_stream_init(shm_stream *stream,
                     size_t event_size,
                     shm_stream_has_event_fn has_event,
                     shm_stream_buffer_events_fn buffer_events,
                     const char * const name);
shm_eventid shm_stream_get_next_id(shm_stream *);
const char *shm_stream_get_name(shm_stream *);

/*
 * Fill the 'dropped' event for the given stream
 */
void shm_stream_get_dropped_event(shm_stream *stream,
                                  shm_event_dropped *dropped_ev,
                                  uint64_t n);

#endif // SHAMON_STREAMS_H
