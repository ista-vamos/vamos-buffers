#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "event.h"
#include "stream.h"

/*****
 * EVENT
 *****/

struct _ev_kind_rec {
    char *name;
    /* copying and destroying the event
       (important if the event carries some
       dynamically allocated memory */
    ev_copy_fn ev_copy;
    ev_destroy_fn ev_destroy;
};

struct _ev_kind_rec *event_kinds = NULL;
size_t ev_kinds_num = 0;
size_t ev_kinds_num_allocated = 0;

shm_kind shm_mk_event_kind(const char* name,
                           ev_copy_fn copy_fn,
                           ev_destroy_fn destroy_fn) {
    size_t idx = ev_kinds_num++;
    if (ev_kinds_num_allocated < ev_kinds_num) {
        ev_kinds_num += 10;
        event_kinds = realloc(event_kinds, sizeof(struct _ev_kind_rec) * ev_kinds_num);
        assert(event_kinds && "Allocation failed");
    }

    // FIXME: we leak these right now, create a destructor or use atexit?
    event_kinds[idx].name = strdup(name);
    event_kinds[idx].ev_copy = copy_fn;
    event_kinds[idx].ev_destroy = destroy_fn;
    assert(event_kinds[idx].name);
    return idx + 1;
};

const char *shm_event_kind_name(shm_kind kind) {
    if (kind > ev_kinds_num)
        return NULL;
    return event_kinds[kind - 1].name;
}

ev_copy_fn shm_event_get_copy_fn(shm_event *ev) {
    shm_kind kind = shm_event_kind(ev);
    if (kind > ev_kinds_num)
        return NULL;
    return event_kinds[kind - 1].ev_copy;
}

ev_destroy_fn shm_event_get_destroy_fn(shm_event *ev) {
    shm_kind kind = shm_event_kind(ev);
    if (kind > ev_kinds_num)
        return NULL;
    return event_kinds[kind - 1].ev_destroy;
}

shm_eventid shm_event_id(shm_event *event) {
	return event->id;
}

shm_kind shm_event_kind(shm_event *event) {
	return event->kind;
};

size_t shm_event_size(shm_event *event) {
    return event->size;
};

shm_stream *shm_event_get_stream(shm_event *event) {
    return event->stream;
}


/*
shm_value shm_event_get_value(shm_event *event, shm_kind field) {
}
*/
