#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "eventinterface.h"
#include "streams.h"

typedef struct _shm_event_io {
	// mandatory
	shm_event base;
	// data
	int fd;
	shm_string_ref str_ref;
} shm_event_io;

typedef struct _shm_stream_io {
	shm_stream base;
    pid_t pid;
    // the kind that we assign to events. If a stream can have only one
    // kind of events, we can move this to base (derive it from the stream name)
    shm_kind ev_kind_in;
    shm_kind ev_kind_out;
} shm_stream_io;

shm_stream *shm_create_io_stream(pid_t pid);
void shm_destroy_io_stream(shm_stream_io *s);

//void shm_destroy_io_stream(shm_stream_stdin *ss);

