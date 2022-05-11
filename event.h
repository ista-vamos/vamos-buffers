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
} shm_event;

typedef struct _shm_event_dropped {
    shm_event base;
    shm_stream *stream;  /* on what stream */
    uint64_t n;          /* how many events were dropped */
} shm_event_dropped;

/* Must be called before using event API.
 * It is called from shamon_create */
void initialize_events(void);
/* called from shamon_destroy */
void deinitialize_events(void);

//Returns some form of identifier represents a particular name,
//used for event kinds, field names, etc. (so we don't need to do string
//comparisons everywhere)
shm_kind shm_mk_event_kind(const char* name,
                           size_t event_size,
                           const char *signature);
const char *shm_event_kind_name(shm_kind kind);

//EVENTS
shm_eventid shm_event_id(shm_event *event);
size_t shm_event_size(shm_event *event);
shm_kind shm_event_kind(shm_event *event);
size_t shm_event_size_for_kind(shm_kind kind);
shm_stream *shm_event_stream(shm_event *event);
const char *shm_event_signature(shm_event *event);

// DROP EVENT
bool shm_event_is_dropped(shm_event *);
shm_kind shm_get_dropped_kind(void);

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
