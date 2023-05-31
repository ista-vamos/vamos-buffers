#include "vamos-buffers/core/event.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/** The kind of `hole` event that represents a gap **/
static const vms_kind hole_kind = 1;
/** The kind of `done` event that represents the end of stream **/
static const vms_kind done_kind = 2;
static const vms_kind last_special_kind = 9;

void initialize_events(void) {
    assert(hole_kind == 1 && "We assume that the 'hole_kind' is 1 for now");
    assert(hole_kind > 0 && "Events not initialized");
}

void deinitialize_events(void) {}

bool vms_event_is_hole(const vms_event *ev) {
    assert(hole_kind > 0);
    return vms_event_kind(ev) == hole_kind;
}

bool vms_event_is_done(const vms_event *ev) {
    assert(done_kind > 0);
    return vms_event_kind(ev) == done_kind;
}

vms_kind vms_get_hole_kind(void) {
    assert(hole_kind > 0);
    assert(last_special_kind >= hole_kind);
    return hole_kind;
}

vms_kind vms_get_done_kind(void) {
    assert(done_kind > 0);
    assert(last_special_kind >= done_kind);
    return done_kind;
}

vms_kind vms_get_last_special_kind(void) {
    assert(last_special_kind > 0);
    return last_special_kind;
}

vms_eventid vms_event_id(const vms_event *event) { return event->id; }

vms_kind vms_event_kind(const vms_event *event) { return event->kind; }
