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
};

}  // namespace vamos

#endif
