#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "event.h"
#include "stream.h"

typedef struct _vms_event_stdin {
    // mandatory
    vms_event base;
    // data
    int fd;
    uint64_t time;
    vms_string_ref str_ref;
} vms_event_stdin;

typedef struct _vms_stream_stdin {
    vms_stream base;
    // user can add here some auxiliary data if needed
    // ...
    // the pointer to the read lines
    char *line;
    size_t line_len;
    // the kind that we assign to events. If a stream can have only one
    // kind of events, we can move this to base (derive it
    // from the stream name)
    vms_kind ev_kind;
    vms_eventid last_event_id;
} vms_stream_stdin;

bool stdin_has_event(vms_stream *stream);

size_t stdin_buffer_events(vms_stream *stream, vms_arbiter_buffer *buffer);

vms_stream *vms_create_stdin_stream();

void vms_destroy_stdin_stream(vms_stream_stdin *ss);
