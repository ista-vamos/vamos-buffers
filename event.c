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
    size_t ev_size;
};

static struct _ev_kind_rec *events_info = NULL;
static size_t ev_kinds_num = 0;
static size_t ev_kinds_num_allocated = 0;
static shm_kind dropped_kind;

shm_kind shm_mk_event_kind(const char* name,
                           size_t event_size) {
    size_t idx = ev_kinds_num++;
    if (ev_kinds_num_allocated < ev_kinds_num) {
        ev_kinds_num_allocated += 10;
        events_info = realloc(events_info, sizeof(struct _ev_kind_rec) * ev_kinds_num_allocated);
        assert(events_info && "Allocation failed");
    }

    events_info[idx].name = strdup(name);
    events_info[idx].ev_size = event_size;
    assert(events_info[idx].name);
    return idx + 1;
};

void initialize_events() {
    if (events_info)
        return;  /* events are initialized */

    dropped_kind = shm_mk_event_kind("dropped",
                                     sizeof(shm_event_dropped));

    assert(dropped_kind > 0 && "Events not initialized");
    assert(events_info && "Events not initialized");
}

void deinitialize_events() {
    for (size_t i = 0; i < ev_kinds_num; ++i) {
        free(events_info[i].name);
    }
    free(events_info);

}

bool shm_event_is_dropped(shm_event *ev) {
    assert(dropped_kind > 0 && "Events not initialized");
    assert(events_info && "Events not initialized");
    assert(shm_event_kind(ev) <= ev_kinds_num && "Invalid event kind");
    return shm_event_kind(ev) == dropped_kind;
}

shm_kind shm_get_dropped_kind() {
    assert(dropped_kind > 0 && "Events not initialized");
    assert(events_info && "Events not initialized");
    return dropped_kind;
}

const char *shm_event_kind_name(shm_kind kind) {
    assert(events_info && "Events not initialized");
    assert(kind <= ev_kinds_num && "Invalid event kind");
    return events_info[kind - 1].name;
}

size_t shm_event_kind_size(shm_kind kind) {
    assert(events_info && "Events not initialized");
    assert(kind <= ev_kinds_num && "Invalid event kind");
    return events_info[kind - 1].ev_size;
};

size_t shm_event_size(shm_event *ev) {
    return shm_event_kind_size(shm_event_kind(ev));
};

shm_eventid shm_event_id(shm_event *event) {
    return event->id;
}

shm_kind shm_event_kind(shm_event *event) {
    return event->kind;
};
