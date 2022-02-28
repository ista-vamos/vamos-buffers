#include <inttypes.h>
#include <stdlib.h>

typedef intptr_t shm_eventqueue;

typedef intptr_t shm_event;

typedef intptr_t shm_constraint;

typedef intptr_t shm_identifier;

typedef intptr_t shm_timestamp;

typedef intptr_t shm_eventid;

//abstract, depends on the kinds of values we'll support. for now, basically just integers, maybe booleans
typedef intptr_t shm_value;

//Returns some form of identifier represents a particular name, used for event kinds, field names, etc. (so we don't need to do string comparisons everywhere)
shm_identifier shm_mk_identifier(char* name);

// special identifiers to mark special event kinds
shm_identifier shm_get_hole_id();
shm_identifier shm_get_end_of_stream_id();

// CONSTRAINTS
shm_constraint shm_constraint_mk_true();

//event.fieldid == value
shm_constraint shm_constraint_mk_eq_int(shm_identifier fieldid, uint64_t value);

//event.kind == id
shm_constraint shm_constraint_mk_kind(shm_identifier kind);

//basic constraint combinators
shm_constraint shm_constraint_mk_not(shm_constraint constr);
shm_constraint shm_constraint_mk_and(shm_constraint left, shm_constraint right);
shm_constraint shm_constraint_mk_or(shm_constraint left, shm_constraint right);


//QUEUES

//creates new event queue
shm_eventqueue shm_eventqueue_create(shm_constraint constraint);

//stops recording events, but events can still be retrieved
void shm_eventqueue_stop(shm_eventqueue queue);

//releases all resources associated with queue
void shm_eventqueue_close(shm_eventqueue queue);

//blocks while waiting; returns event with kind shm_get_end_of_stream_id() at end of stream
shm_event get_next_event(shm_eventqueue queue);

//EVENTS
shm_eventid shm_event_id(shm_event event);
shm_timestamp shm_event_timestamp(shm_event event);
shm_identifier shm_event_kind(shm_event event);
shm_value shm_event_get_value(shm_event event, shm_identifier field);

//TIMESTAMPS
// have numerical upper and lower bounds, and possibly also pre-orderings w.r.t. other timestamps (-1 = less than, 0 = equal, 1 = greater than)
uint64_t shm_timestamp_lb(shm_timestamp ts);
uint64_t shm_timestamp_ub(shm_timestamp ts);
size_t shm_timestamp_additional_bound_count(shm_timestamp ts);
size_t shm_timestamp_additional_bound_value(shm_timestamp ts, size_t index);
int shm_timestamp_additional_bound_kind(shm_timestamp ts, size_t index);

//VALUES
int64_t shm_value_get_int64(shm_value value);
int shm_value_get_boolean(shm_value value);
