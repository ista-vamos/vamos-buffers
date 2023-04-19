
#include "vamos-buffers/streams/stream-regexrw.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "vamos-buffers/core/arbiter.h"

bool sregexrw_is_ready(vms_stream *stream) {
    struct buffer *b = ((vms_stream_sregexrw *)stream)->shmbuffer;
    /* buffer must be ready or it may not be ready anymore, but it
     * still has some data that we haven't read */
    return buffer_is_ready(b) || buffer_size(b) > 0;
}

void sregexrw_alter(vms_stream *stream, vms_event *in, vms_event *out) {
    memcpy(out, in, stream->event_size);
}

vms_stream *vms_create_sregexrw_stream(const char *key, const char *name) {
    vms_stream_sregexrw *ss = malloc(sizeof *ss);
    struct buffer *shmbuffer = get_shared_buffer(key);
    assert(shmbuffer && "Getting the shm buffer failed");
    size_t elem_size = buffer_elem_size(shmbuffer);
    assert(elem_size > 0);
    vms_stream_init((vms_stream *)ss, shmbuffer, elem_size, sregexrw_is_ready,
                    NULL, sregexrw_alter, NULL, NULL, "regexrw-stream", name);
    ss->shmbuffer = shmbuffer;

    return (vms_stream *)ss;
}
