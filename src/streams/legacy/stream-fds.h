#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "event.h"
#include "queue.h"
#include "stream.h"

typedef struct _vms_event_fd_in {
    // mandatory
    vms_event base;
    // data
    int fd;
    uint64_t time;
    vms_string_ref str_ref;
} vms_event_fd_in;

typedef struct _vms_stream_fds {
    vms_stream base;
    // user can add here some auxiliary data if needed
    // ...
    // the pointer to the read lines
    char *line;
    // the kind that we assign to events. If a stream can have only one
    // kind of events, we can move this to base (derive it
    // from the stream name)
    vms_kind ev_kind_in;
    // filedescriptors that we track
    struct pollfd *fds;
    // allocated size
    size_t fds_size;
    // number of filedescriptors
    size_t fds_num;
    // buffer for each fd for reading
    vms_string *fds_buffer;
    // events to be pushed to the monitor
    vms_queue pending_events;
} vms_stream_fds;

vms_stream *vms_create_fds_stream();
void vms_stream_fds_add_fd(vms_stream_fds *stream, int fd);

void vms_destroy_fds_stream(vms_stream_fds *ss);
