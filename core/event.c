#include "event.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "stream.h"

static const shm_kind hole_kind = 1;
static const shm_kind last_special_kind = 1;

void initialize_events() {
    assert(hole_kind == 1 && "We assume that the 'hole_kind' is 1 for now");
    assert(hole_kind > 0 && "Events not initialized");
}

void deinitialize_events() {}

bool shm_event_is_hole(shm_event *ev) {
    assert(hole_kind > 0);
    return shm_event_kind(ev) == hole_kind;
}

shm_kind shm_get_hole_kind() {
    assert(hole_kind > 0);
    assert(last_special_kind >= hole_kind);
    return hole_kind;
}

shm_kind shm_get_last_special_kind() {
    assert(last_special_kind > 0);
    return last_special_kind;
}

shm_eventid shm_event_id(shm_event *event) { return event->id; }

shm_kind shm_event_kind(shm_event *event) { return event->kind; }
