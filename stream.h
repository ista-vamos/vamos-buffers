#ifndef SHAMON_STREAMS_H
#define SHAMON_STREAMS_H

#include "event.h"
#include "source.h"

typedef struct _shm_arbiter_buffer shm_arbiter_buffer;

typedef size_t (*shm_stream_buffer_events_fn)(struct _shm_stream *,
                                              shm_arbiter_buffer *buffer);
typedef bool (*shm_stream_is_ready_fn)(struct _shm_stream *);
typedef void (*shm_stream_destroy_fn)(struct _shm_stream *);

typedef bool (*shm_stream_filter_fn)(shm_stream *, shm_event *);
typedef void (*shm_stream_alter_fn)(shm_stream *, shm_event *, shm_event *);

// TODO: make this opaque
typedef struct _shm_stream {
    uint64_t id;
    const char *name;
    size_t event_size;
    struct buffer *incoming_events;
    /* callbacks */
    shm_stream_is_ready_fn is_ready;
    shm_stream_filter_fn filter;
    shm_stream_alter_fn alter;
    shm_stream_destroy_fn destroy;
#ifndef NDEBUG
    /* for checking consistency */
    size_t last_event_id;
#endif
#ifdef DUMP_STATS
   size_t read_events;
   size_t fetched_events;
   size_t consumed_events;
   size_t dropped_events;
   size_t slept_waiting_for_ev;
#endif
} shm_stream;

void shm_stream_init(shm_stream *stream,
                     struct buffer *incoming_events,
                     size_t event_size,
                     shm_stream_is_ready_fn is_ready,
                     shm_stream_filter_fn filter,
                     shm_stream_alter_fn alter,
                     shm_stream_destroy_fn destroy,
                     const char * const name);

const char *shm_stream_get_name(shm_stream *);
size_t shm_stream_event_size(shm_stream *);
size_t shm_stream_id(shm_stream *);
struct event_record *shm_stream_get_avail_events(shm_stream *, size_t *);

void *shm_stream_read_events(shm_stream *, size_t *);
bool shm_stream_consume(shm_stream *stream, size_t num);
const char *shm_stream_get_str(shm_stream *stream, uint64_t elem);

void shm_stream_notify_last_processed_id(shm_stream *stream, shm_eventid id);

/*
 * Fill the 'dropped' event for the given stream
 */
void shm_stream_get_dropped_event(shm_stream *stream,
                                  shm_event_dropped *dropped_ev,
                                  size_t id,
                                  uint64_t n);

bool shm_stream_is_ready(shm_stream *);

void shm_stream_notify_dropped(shm_stream *stream,
                               uint64_t begin_id,
                               uint64_t end_id);
void shm_stream_destroy(shm_stream *stream);

#ifndef NDEBUG
inline size_t shm_stream_last_event_id(shm_stream *s) {
    return s->last_event_id;
}
#endif
#endif // SHAMON_STREAMS_H
