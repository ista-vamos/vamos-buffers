#include "vamos-buffers/streams/stream-drregex.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "vamos-buffers/core/arbiter.h"
#include "vamos-buffers/shmbuf/buffer.h"

bool drregex_is_ready(vms_stream *stream) {
    struct buffer *b = ((vms_stream_drregex *)stream)->shmbuffer;
    /* buffer must be ready or it may not be ready anymore, but it
     * still has some data that we haven't read */
    return buffer_is_ready(b) || buffer_size(b) > 0;
}

void drregex_alter(vms_stream *stream, vms_event *in, vms_event *out) {
    memcpy(out, in, stream->event_size);
}

vms_stream *vms_create_drregex_stream(const char *key, const char *name) {
    vms_stream_drregex *ss = malloc(sizeof *ss);
    struct buffer *shmbuffer = get_shared_buffer(key);
    assert(shmbuffer && "Getting the shm buffer failed");
    size_t elem_size = buffer_elem_size(shmbuffer);
    assert(elem_size > 0);
    vms_stream_init((vms_stream *)ss, shmbuffer, elem_size, drregex_is_ready,
                    NULL, drregex_alter, NULL, NULL, "drregex-stream", name);
    ss->shmbuffer = shmbuffer;

    return (vms_stream *)ss;
}
