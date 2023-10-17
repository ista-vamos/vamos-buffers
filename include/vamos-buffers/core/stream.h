#ifndef VAMOS_STREAMS_H
#define VAMOS_STREAMS_H

#include "event.h"
#include "source.h"
#include "vector-macro.h"

typedef struct _vms_arbiter_buffer vms_arbiter_buffer;

typedef size_t (*vms_stream_buffer_events_fn)(struct _vms_stream *,
                                              vms_arbiter_buffer *buffer);
typedef bool (*vms_stream_is_ready_fn)(struct _vms_stream *);
typedef void (*vms_stream_destroy_fn)(struct _vms_stream *);

typedef bool (*vms_stream_filter_fn)(vms_stream *, vms_event *);
typedef void (*vms_stream_alter_fn)(vms_stream *, vms_event *, vms_event *);
typedef void (*vms_stream_hole_init_fn)(vms_event *);
typedef void (*vms_stream_hole_update_fn)(vms_event *, vms_event *);

typedef struct _vms_stream_hole_handling {
    size_t hole_event_size;
    vms_stream_hole_init_fn init;
    vms_stream_hole_update_fn update;
} vms_stream_hole_handling;

// TODO: make this opaque
typedef struct _vms_stream {
    uint64_t id;
    char *name;
    char *type;
    size_t event_size;
    /* shared-memory buffer */
    struct buffer *incoming_events_buffer;
    /* the number of created substreams (sub-buffers) for the
     * shared memory buffer */
    size_t substreams_no;
    /* cached info about events in 'incoming_events_buffer' */
    struct event_record *events_cache;
    /* callbacks */
    vms_stream_is_ready_fn is_ready;
    vms_stream_filter_fn filter;
    vms_stream_alter_fn alter;
    vms_stream_destroy_fn destroy;
    vms_stream_hole_handling hole_handling;
    /* substreams of this stream and the link to the parent */
    vms_stream *parent_stream;
    VEC(substreams, struct _vms_stream *);
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
} vms_stream;

void vms_stream_init(vms_stream *stream, struct buffer *incoming_events_buffer,
                     size_t event_size, vms_stream_is_ready_fn is_ready,
                     vms_stream_filter_fn filter, vms_stream_alter_fn alter,
                     vms_stream_destroy_fn destroy,
                     const vms_stream_hole_handling *hole_handling,
                     const char *const type, const char *const name);

const char *vms_stream_get_name(vms_stream *);
const char *vms_stream_get_type(vms_stream *);
size_t vms_stream_event_size(vms_stream *);
size_t vms_stream_id(vms_stream *);

struct event_record *vms_stream_get_avail_events(vms_stream *, size_t *);
struct event_record *vms_stream_get_event_record(vms_stream *, vms_kind);
struct event_record *vms_stream_get_event_record_no_cache(vms_stream *,
                                                          vms_kind);

/* register to receiving events particular events */
int vms_stream_register_event(vms_stream *, const char *, vms_kind);
int vms_stream_register_events(vms_stream *, size_t sz, ...);
int vms_stream_register_all_events(vms_stream *);

_Bool vms_stream_has_new_substreams(vms_stream *stream);
_Bool vms_stream_is_substream(vms_stream *stream);

vms_stream *vms_stream_create_substream(
    vms_stream *stream, vms_stream_is_ready_fn is_ready,
    vms_stream_filter_fn filter, vms_stream_alter_fn alter,
    vms_stream_destroy_fn destroy, vms_stream_hole_handling *hole_handling);

/* the number of elements in the (shared memory) buffer of the stream */
size_t vms_stream_buffer_size(vms_stream *);
/* the capacity the (shared memory) buffer of the stream */
size_t vms_stream_buffer_capacity(vms_stream *);

void *vms_stream_read_events(vms_stream *, size_t *);
bool vms_stream_consume(vms_stream *stream, size_t num);
const char *vms_stream_get_str(vms_stream *stream, uint64_t elem);

void vms_stream_notify_last_processed_id(vms_stream *stream, vms_eventid id);
bool vms_stream_is_ready(vms_stream *);
void vms_stream_attach(vms_stream *stream);
void vms_stream_detach(vms_stream *stream);

void vms_stream_prepare_hole_event(vms_stream *stream, vms_event *ev, size_t id,
                                   uint64_t n);

/*
 * returns true if there will come no events in the future, false otherwise
 * */
bool vms_stream_is_finished(vms_stream *);

void vms_stream_notify_dropped(vms_stream *stream, uint64_t begin_id,
                               uint64_t end_id);
void vms_stream_destroy(vms_stream *stream);

#ifndef NDEBUG
inline size_t vms_stream_last_event_id(vms_stream *s) {
    return s->last_event_id;
}
#endif

void vms_stream_dump_events(vms_stream *stream);
#endif  // VAMOS_STREAMS_H
