#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "vamos-buffers/core/event.h"

static const vms_kind hole_kind = 1;
static const vms_kind last_special_kind = 1;

void initialize_events(void) {
    assert(hole_kind == 1 && "We assume that the 'hole_kind' is 1 for now");
    assert(hole_kind > 0 && "Events not initialized");
}

void deinitialize_events(void) {}

bool vms_event_is_hole(vms_event *ev) {
    assert(hole_kind > 0);
    return vms_event_kind(ev) == hole_kind;
}

vms_kind vms_get_hole_kind(void) {
    assert(hole_kind > 0);
    assert(last_special_kind >= hole_kind);
    return hole_kind;
}

vms_kind vms_get_last_special_kind(void) {
    assert(last_special_kind > 0);
    return last_special_kind;
}

vms_eventid vms_event_id(vms_event *event) { return event->id; }

vms_kind vms_event_kind(vms_event *event) { return event->kind; }
