#include "stream-stdin.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "arbiter.h"

bool stdin_is_ready(shm_stream *stream) { return true; }

void stdin_copy_event(shm_stream *stream, shm_event *in, shm_event *out) {
    static shm_event_stdin ev;
    shm_stream_stdin *ss = (shm_stream_stdin *)stream;
    ssize_t len = getline(&ss->line, &ss->line_len, stdin);

    // TODO: return end-of-stream event
    if (len == -1)
        return 0;

    ev.time = clock();
    // ev.base.stream = stream;
    ev.base.kind = ss->ev_kind;
    ev.base.id = ++ss->last_event_id;
    ev.fd = fileno(stdin);
    ev.str_ref.size = len;
    ev.str_ref.data = ss->line;

    shm_arbiter_buffer_push(buffer, &ev, sizeof(ev));
    return 1;
}

shm_stream *shm_create_stdin_stream() {
    shm_stream_stdin *ss = malloc(sizeof *ss);
    shm_stream_init((shm_stream *)ss, sizeof(shm_event_stdin), stdin_is_ready,
                    /* filter */ NULL, copy_event, "stdin-stream");
    ss->line = NULL;
    ss->line_len = 0;
    ss->ev_kind = shm_mk_event_kind("stdin", (shm_stream *)ss,
                                    sizeof(shm_event_stdin), NULL, NULL);
    return (shm_stream *)ss;
}

void shm_destroy_stdin_stream(shm_stream_stdin *ss) {
    free(ss->line);
    free(ss);
}
