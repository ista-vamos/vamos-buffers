#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "event.h"
#include "queue.h"
#include "stream.h"

typedef struct _shm_event_fd_in {
    // mandatory
    shm_event base;
    // data
    int fd;
    uint64_t time;
    shm_string_ref str_ref;
} shm_event_fd_in;

typedef struct _shm_stream_fds {
    shm_stream base;
    // user can add here some auxiliary data if needed
    // ...
    // the pointer to the read lines
    char *line;
    // the kind that we assign to events. If a stream can have only one
    // kind of events, we can move this to base (derive it
    // from the stream name)
    shm_kind ev_kind_in;
    // filedescriptors that we track
    struct pollfd *fds;
    // allocated size
    size_t fds_size;
    // number of filedescriptors
    size_t fds_num;
    // buffer for each fd for reading
    shm_string *fds_buffer;
    // events to be pushed to the monitor
    shm_queue pending_events;
} shm_stream_fds;

shm_stream *shm_create_fds_stream();
void shm_stream_fds_add_fd(shm_stream_fds *stream, int fd);

void shm_destroy_fds_stream(shm_stream_fds *ss);
