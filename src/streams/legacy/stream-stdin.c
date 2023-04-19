#include "stream-stdin.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "arbiter.h"

bool stdin_is_ready(vms_stream *stream) { return true; }

void stdin_copy_event(vms_stream *stream, vms_event *in, vms_event *out) {
    static vms_event_stdin ev;
    vms_stream_stdin *ss = (vms_stream_stdin *)stream;
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

    vms_arbiter_buffer_push(buffer, &ev, sizeof(ev));
    return 1;
}

vms_stream *vms_create_stdin_stream() {
    vms_stream_stdin *ss = malloc(sizeof *ss);
    vms_stream_init((vms_stream *)ss, sizeof(vms_event_stdin), stdin_is_ready,
                    /* filter */ NULL, copy_event, "stdin-stream");
    ss->line = NULL;
    ss->line_len = 0;
    ss->ev_kind = vms_mk_event_kind("stdin", (vms_stream *)ss,
                                    sizeof(vms_event_stdin), NULL, NULL);
    return (vms_stream *)ss;
}

void vms_destroy_stdin_stream(vms_stream_stdin *ss) {
    free(ss->line);
    free(ss);
}
