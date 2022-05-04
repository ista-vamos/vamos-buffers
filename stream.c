#include <assert.h>

#include "stream.h"
#include "shmbuf/buffer.h"

/*****
 * STREAM
 *****/

const char *shm_stream_get_name(shm_stream *stream) {
    assert(stream);
    return stream->name;
}

static uint64_t last_stream_id = 0;

void shm_stream_init(shm_stream *stream,
                     struct buffer *incoming_events,
                     size_t event_size,
                     shm_stream_is_ready_fn is_ready,
                     shm_stream_filter_fn filter,
                     shm_stream_alter_fn alter,
                     const char * const name) {
        stream->id = ++last_stream_id;
        stream->event_size = event_size;
        stream->incoming_events = incoming_events;
        stream->is_ready = is_ready;
        stream->filter = filter;
        stream->alter = alter;
        stream->name = name;
}

// not thread safe!
void shm_stream_get_dropped_event(shm_stream *stream,
                                  shm_event_dropped *dropped_ev,
                                  size_t id,
                                  uint64_t n) {
    dropped_ev->base.id = id;
    dropped_ev->base.kind = shm_get_dropped_kind();
    dropped_ev->stream = stream;
    dropped_ev->n = n;
}

bool shm_stream_is_ready(shm_stream *s) {
    return s->is_ready(s);
}

void *shm_stream_read_events(shm_stream *s, size_t *num) {
    assert(shm_stream_is_ready(s));
    return buffer_read_pointer(s->incoming_events, num);
}

bool shm_stream_consume(shm_stream *stream, size_t num) {
    return buffer_drop_k(stream->incoming_events, num);
}


void shm_buffer_release_str(shm_stream *stream,
                            uint64_t elem) {
    buffer_release_str(stream->incoming_events, elem);
}

const char *shm_stream_get_str(shm_stream *stream, uint64_t elem) {
    return buffer_get_str(stream->incoming_events, elem);
}
