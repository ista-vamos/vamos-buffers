#ifndef SHAMON_EVENTINTERFACE_H_
#define SHAMON_EVENTINTERFACE_H_

#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>

// typedef intptr_t shm_eventqueue;
//QUEUES

//creates new event queue
// shm_eventqueue shm_eventqueue_create(shm_constraint constraint);

//stops recording events, but events can still be retrieved
// void shm_eventqueue_stop(shm_eventqueue queue);

//releases all resources associated with queue
// void shm_eventqueue_close(shm_eventqueue queue);

//blocks while waiting; returns event with kind shm_get_end_of_stream_id() at end of stream
// shm_event get_next_event(shm_eventqueue queue);


typedef uint64_t shm_kind;

typedef uint64_t shm_eventid;

struct _shm_stream;
typedef struct _shm_stream shm_stream;

// TODO: make this opaque? but how to do the inheritance, then? Via
// pointers to an interface?
typedef struct _shm_event {
    size_t size;                // must be set for each event type
    shm_kind kind;
    shm_eventid id;
    struct _shm_stream *stream;
} shm_event;

//Returns some form of identifier represents a particular name,
//used for event kinds, field names, etc. (so we don't need to do string
//comparisons everywhere)
shm_kind shm_mk_event_kind(const char* name);
const char *shm_kind_get_name(shm_kind kind);

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

// CONSTRAINTS
//  To be implemented independently in monitors
/*
typedef intptr_t shm_constraint;

shm_constraint shm_constraint_mk_true();

//event.fieldid == value
shm_constraint shm_constraint_mk_eq_int(shm_kind fieldid, uint64_t value);

//event.kind == id
shm_constraint shm_constraint_mk_kind(shm_kind kind);

//basic constraint combinators
shm_constraint shm_constraint_mk_not(shm_constraint constr);
shm_constraint shm_constraint_mk_and(shm_constraint left, shm_constraint right);
shm_constraint shm_constraint_mk_or(shm_constraint left, shm_constraint right);
*/



//VALUES -- will be handled by inheritance
//abstract, depends on the kinds of values we'll support. for now, basically
//just integers, maybe booleans
// typedef intptr_t shm_value;
/*
int64_t shm_value_get_int64(shm_value value);
int shm_value_get_boolean(shm_value value);
//shm_value shm_event_get_value(shm_event *event, shm_kind field);
*/


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
