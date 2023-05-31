#ifndef VAMOS_BUFFERS_EVENT_H
#define VAMOS_BUFFERS_EVENT_H

#include <vamos-buffers/core/event.h>

namespace vamos {

struct Event : public vms_event {
    Event() {}
    Event(vms_kind kind, vms_eventid id) : _vms_event(kind, id) {}

    vms_eventid id() const { return vms_event_id(this); }
    vms_kind kind() const { return vms_event_kind(this); };
    bool is_hole() const { return vms_event_is_hole(this); }
    bool is_done() const { return vms_event_is_done(this); }

    static Event getDone(vms_eventid id) {
        return Event(vms_event_get_done_kind(), id);
    }

    static constexpr vms_kind holeKind() {
        return VMS_EVENT_HOLE_KIND;
    }

    static constexpr vms_kind doneKind() {
        return VMS_EVENT_DONE_KIND;
    }

    static constexpr vms_kind lastSpecialKind() {
        return VMS_EVENT_LAST_SPECIAL_KIND;
    }

    static constexpr vms_kind firstValidKind() {
        return VMS_EVENT_LAST_SPECIAL_KIND + 1;
    }
};

}  // namespace vamos

#endif
