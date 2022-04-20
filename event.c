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
};

struct _ev_kind_rec *event_kinds = NULL;
size_t ev_kinds_num = 0;
size_t ev_kinds_num_allocated = 0;

shm_kind shm_mk_event_kind(const char* name) {
    size_t idx = ev_kinds_num++;
    if (ev_kinds_num_allocated < ev_kinds_num) {
        ev_kinds_num += 10;
        event_kinds = realloc(event_kinds, sizeof(struct _ev_kind_rec) * ev_kinds_num);
        assert(event_kinds && "Allocation failed");
    }

    // FIXME: we leak these right now, create a destructor or use atexit?
    event_kinds[idx].name = strdup(name);
    assert(event_kinds[idx].name);
    return idx + 1;
};

const char *shm_kind_get_name(shm_kind kind) {
    if (kind > ev_kinds_num)
        return NULL;
    return event_kinds[kind - 1].name;
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
