#ifndef VMS_EVENT_H_
#define VMS_EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

typedef uint64_t vms_kind;
typedef uint64_t vms_eventid;
typedef struct _vms_stream vms_stream;

// TODO: make this opaque? but how to do the inheritance, then? Via
// pointers to an interface?
typedef struct _vms_event {
    vms_kind kind;
    vms_eventid id;

#ifdef __cplusplus
    _vms_event() {}
    _vms_event(vms_kind kind, vms_eventid id) : kind(kind), id(id) {}
#endif
} vms_event;

typedef struct _vms_event_default_hole {
    vms_event base;
    size_t n; /* number of dropped events */
} vms_event_default_hole;

/* Must be called before using event API.
 * It is called from shamon_create */
void initialize_events(void) __attribute__((deprecated));
/* called from shamon_destroy */
void deinitialize_events(void) __attribute__((deprecated));

// EVENTS
vms_eventid vms_event_id(const vms_event *event);
vms_kind vms_event_kind(const vms_event *event);

// DROP EVENT
bool vms_event_is_hole(const vms_event *);
vms_kind vms_get_hole_kind(void);
vms_kind vms_get_last_special_kind(void);

#ifdef __cplusplus
}
#endif

#endif  // VMS_EVENT_H
