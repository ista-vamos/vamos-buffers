#ifndef VAMOS_BUFFERS_EVENT_H
#define VAMOS_BUFFERS_EVENT_H

#include <vamos-buffers/core/event.h>

namespace vamos {

struct Event : public vms_event {
    Event() {}
    Event(vms_kind kind) : _vms_event(kind) {}
    Event(vms_kind kind, vms_eventid id) : _vms_event(kind, id) {}

    bool is_hole() const { return vms_event_is_hole(this); }
    bool is_done() const { return vms_event_is_done(this); }
    void set_id(vms_eventid id) { vms_event_set_id(this, id); }

    static Event getDone(vms_eventid id) {
        return Event(vms_event_get_done_kind(), id);
    }

    static constexpr vms_kind holeKind() { return VMS_EVENT_HOLE_KIND; }

    static constexpr vms_kind doneKind() { return VMS_EVENT_DONE_KIND; }

    static constexpr vms_kind lastSpecialKind() {
        return VMS_EVENT_LAST_SPECIAL_KIND;
    }

    static constexpr vms_kind firstValidKind() {
        return VMS_EVENT_LAST_SPECIAL_KIND + 1;
    }
};

}  // namespace vamos

#endif
