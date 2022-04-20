#ifndef SHAMON_STREAMS_H
#define SHAMON_STREAMS_H

#include "event.h"

typedef bool (*shm_stream_has_event_fn)(struct _shm_stream *);
typedef shm_event *(*shm_stream_get_next_event_fn)(struct _shm_stream *);
// TODO: make this opaque
typedef struct _shm_stream {
    uint64_t id;
    const char *name;
    shm_eventid last_event_id;
    size_t event_size;
    shm_stream_has_event_fn has_event;
    shm_stream_get_next_event_fn get_next_event;
} shm_stream;

void shm_stream_init(shm_stream *stream,
                     size_t event_size,
                     shm_stream_has_event_fn has_event,
                     shm_stream_get_next_event_fn get_next_event,
                     const char * const name);
shm_eventid shm_stream_get_next_id(shm_stream *);
shm_event *shm_stream_get_next_ev(shm_stream *);
const char *shm_stream_get_name(shm_stream *);

#endif // SHAMON_STREAMS_H
