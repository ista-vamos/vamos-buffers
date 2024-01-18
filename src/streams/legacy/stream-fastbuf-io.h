#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "event.h"
#include "stream.h"

typedef struct _vms_event_io {
    // mandatory
    vms_event base;
    // data
    int fd;
    uint64_t time;
    vms_string_ref str_ref;
} vms_event_io;

typedef struct _vms_stream_io {
    vms_stream base;
    pid_t pid;
    // the kind that we assign to events. If a stream can have only one
    // kind of events, we can move this to base (derive it from the stream name)
    vms_kind ev_kind_in;
    vms_kind ev_kind_out;
} vms_stream_io;

vms_stream *vms_create_io_stream(pid_t pid);
void vms_destroy_io_stream(vms_stream_io *s);

// void vms_destroy_io_stream(vms_stream_stdin *ss);
