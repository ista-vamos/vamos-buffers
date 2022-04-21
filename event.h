#ifndef SHAMON_EVENTINTERFACE_H_
#define SHAMON_EVENTINTERFACE_H_

#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>

typedef uint64_t shm_kind;

typedef uint64_t shm_eventid;

typedef struct _shm_stream shm_stream;

// TODO: make this opaque? but how to do the inheritance, then? Via
// pointers to an interface?
typedef struct _shm_event {
    shm_kind kind;
    shm_eventid id;
    struct _shm_stream *stream;
} shm_event;

typedef void (*ev_copy_fn) (shm_event *src, shm_event *dst);
typedef void (*ev_destroy_fn) (shm_event *src);

//Returns some form of identifier represents a particular name,
//used for event kinds, field names, etc. (so we don't need to do string
//comparisons everywhere)
shm_kind shm_mk_event_kind(const char* name,
                           size_t event_size,
                           ev_copy_fn copy_fn,
                           ev_destroy_fn destroy_fn);
const char *shm_event_kind_name(shm_kind kind);

// special kinds to mark special event kinds: hole and end of stream
shm_kind shm_get_hole_kind();
shm_kind shm_get_eos_kind();

//EVENTS
// void shm_event_init(size_t size, shm_kind kind, shm_stream *stream);
// void shm_event_init_with_time(size_t size, shm_kind kind, shm_stream *stream);

shm_eventid shm_event_id(shm_event *event);
size_t shm_event_size(shm_event *event);
shm_kind shm_event_kind(shm_event *event);
shm_stream *shm_event_get_stream(shm_event *event);
// full copy of the event including full copy of the data
// (i.e., if the data is a string, the whole string is copied)
// void shm_event_copy(shm_event *event, shm_event *new_event);
// shallow copy of the event, non-primitive data (e.g., strings)
// are only referenced
// TODO: change to reference counting?
// void shm_event_shallow_copy(shm_event *event, shm_event *new_event);
bool shm_event_is_hole(shm_event *);
bool shm_event_is_eos(shm_event *);

// auxiliary structs
typedef struct _shm_string_ref {
    size_t size;
    const char *data;
} shm_string_ref;

typedef struct _shm_string {
    size_t size;
    size_t alloc_size;
    char *data;
} shm_string;

#endif // SHAMON_EVENTINTERFACE_H_
