#include "stream.h"

/*****
 * STREAM
 *****/
shm_eventid shm_stream_get_next_id(shm_stream *stream) {
      return ++stream->last_event_id;
}

const char *shm_stream_get_name(shm_stream *stream) {
    return stream->name;
}

static uint64_t last_stream_id = 0;

void shm_stream_init(shm_stream *stream,
                     size_t event_size,
                     shm_stream_has_event_fn has_event,
                     shm_stream_get_next_event_fn get_next_event,
                     const char * const name) {
        stream->id = ++last_stream_id;
        stream->event_size = event_size;
        stream->get_next_event = get_next_event;
        stream->has_event = has_event;
        stream->last_event_id = 0;
        stream->name = name;
}

// just a comfy fun
shm_event *shm_stream_get_next_ev(shm_stream *stream) {
        return stream->get_next_event(stream);
}

