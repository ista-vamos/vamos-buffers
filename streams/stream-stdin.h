#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "event.h"
#include "stream.h"

typedef struct _shm_event_stdin {
    // mandatory
    shm_event base;
    // data
    int            fd;
    uint64_t       time;
    shm_string_ref str_ref;
} shm_event_stdin;

typedef struct _shm_stream_stdin {
    shm_stream base;
    // user can add here some auxiliary data if needed
    // ...
    // the pointer to the read lines
    char  *line;
    size_t line_len;
    // the kind that we assign to events. If a stream can have only one
    // kind of events, we can move this to base (derive it
    // from the stream name)
    shm_kind    ev_kind;
    shm_eventid last_event_id;
} shm_stream_stdin;

bool stdin_has_event(shm_stream *stream);

size_t stdin_buffer_events(shm_stream *stream, shm_arbiter_buffer *buffer);

shm_stream *shm_create_stdin_stream();

void shm_destroy_stdin_stream(shm_stream_stdin *ss);
