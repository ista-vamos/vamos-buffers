#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "stream-stdin.h"

bool stdin_has_event(shm_stream *stream) {
        //return fseek(stdin, 0, SEEK_END), ftell(stdin) > 0;
    return true;
}

shm_event_stdin *stdin_get_next_event(shm_stream *stream) {
    static shm_event_stdin ev;
    shm_stream_stdin *ss = (shm_stream_stdin *) stream;
    size_t len = 0;
    ev.base.timestamp_lb = (shm_timestamp)clock();
    ssize_t line_len = getline(&ss->line, &len, stdin);
    ev.base.timestamp_ub = (shm_timestamp)clock();

    // TODO: return end-of-stream event
    if (line_len == -1)
        return NULL;

    ev.base.size = sizeof(ev);
    ev.base.stream = stream;
    ev.base.kind = ss->ev_kind;
    ev.base.id = shm_stream_get_next_id(stream);
    ev.fd = fileno(stdin);
    ev.str_ref.size = line_len;
    ev.str_ref.data = ss->line;

    return &ev;
}

shm_stream *shm_create_stdin_stream() {
    shm_stream_stdin *ss = malloc(sizeof *ss);
    shm_stream_init((shm_stream *)ss,
                    (shm_stream_has_event_fn) stdin_has_event,
                    (shm_stream_get_next_event_fn) stdin_get_next_event,
                     "stdin-stream");
    ss->line = NULL;
    ss->ev_kind = shm_mk_event_kind("stdin");
    return (shm_stream *) ss;
}

void shm_destroy_stdin_stream(shm_stream_stdin *ss) {
    free(ss->line);
    free(ss);
}

