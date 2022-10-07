#ifndef SHAMON_EVENT_H_
#define SHAMON_EVENT_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

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
    uint64_t n; /* how many events were dropped */
} shm_event_dropped;

/* Must be called before using event API.
 * It is called from shamon_create */
void initialize_events(void) __attribute__((deprecated));
/* called from shamon_destroy */
void deinitialize_events(void) __attribute__((deprecated));

// EVENTS
shm_eventid shm_event_id(shm_event *event);
shm_kind shm_event_kind(shm_event *event);

// DROP EVENT
bool shm_event_is_dropped(shm_event *);
shm_kind shm_get_dropped_kind(void);
shm_kind shm_get_last_special_kind(void);

#endif // SHAMON_EVENT_H_
