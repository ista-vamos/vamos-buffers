#include <assert.h>

#include "stream.h"

/*****
 * STREAM
 *****/
shm_eventid shm_stream_get_next_id(shm_stream *stream) {
      return ++stream->last_event_id;
}

const char *shm_stream_get_name(shm_stream *stream) {
    assert(stream);
    return stream->name;
}

static uint64_t last_stream_id = 0;

void shm_stream_init(shm_stream *stream,
                     size_t event_size,
                     shm_stream_has_event_fn has_event,
                     shm_stream_buffer_events_fn buffer_events,
                     shm_stream_publish_event_fn publish_event,
                     const char * const name) {
        stream->id = ++last_stream_id;
        stream->event_size = event_size;
        stream->buffer_events = buffer_events;
        stream->has_event = has_event;
        stream->publish_event = publish_event;
        stream->last_event_id = 0;
        stream->name = name;
}

// not thread safe!
void shm_stream_get_dropped_event(shm_stream *stream,
                                  shm_event_dropped *dropped_ev,
                                  uint64_t n) {
    dropped_ev->base.id = shm_stream_get_next_id(stream);
    dropped_ev->base.kind = shm_get_dropped_kind();
    dropped_ev->base.stream = stream;
    dropped_ev->n = n;
}
