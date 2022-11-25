#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "event.h"
#include "stream.h"

static shm_kind dropped_kind      = 1;
static shm_kind last_special_kind = 1;

void initialize_events() {
    assert(dropped_kind == 1 &&
           "We assume that the 'dropped_kind' is 1 for now");
    assert(dropped_kind > 0 && "Events not initialized");
}

void deinitialize_events() {}

bool shm_event_is_dropped(shm_event *ev) {
    assert(dropped_kind > 0);
    return shm_event_kind(ev) == dropped_kind;
}

shm_kind shm_get_dropped_kind() {
    assert(dropped_kind > 0);
    assert(last_special_kind >= dropped_kind);
    return dropped_kind;
}

shm_kind shm_get_last_special_kind() {
    assert(last_special_kind > 0);
    return last_special_kind;
}

shm_eventid shm_event_id(shm_event *event) {
    return event->id;
}

shm_kind shm_event_kind(shm_event *event) {
    return event->kind;
}
