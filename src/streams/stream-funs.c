#include "vamos-buffers/streams/stream-funs.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "vamos-buffers/core/arbiter.h"
#include "vamos-buffers/shmbuf/buffer.h"

bool funs_is_ready(vms_stream *stream) {
    struct buffer *b = ((vms_stream_funs *)stream)->shmbuffer;
    /* buffer must be ready or it may not be ready anymore, but it
     * still has some data that we haven't read */
    return buffer_is_ready(b) || buffer_size(b) > 0;
}

void funs_alter(vms_stream *stream, vms_event *in, vms_event *out) {
    memcpy(out, in, stream->event_size);
}

size_t stream_mk_event_kinds(const char *stream_name, struct buffer *shmbuffer,
                             size_t *max_ev_size);

vms_stream *vms_create_funs_stream(const char *key, const char *name) {
    vms_stream_funs *ss = malloc(sizeof *ss);
    struct buffer *shmbuffer = get_shared_buffer(key);
    assert(shmbuffer && "Getting the shm buffer failed");
    size_t elem_size = buffer_elem_size(shmbuffer);
    assert(elem_size > 0);
    vms_stream_init((vms_stream *)ss, shmbuffer, elem_size, funs_is_ready, NULL,
                    funs_alter, NULL, NULL, "funs-stream", name);
    ss->shmbuffer = shmbuffer;

    ss->ev_buff = NULL;

    return (vms_stream *)ss;
}

const char *vms_stream_funs_get_str(vms_stream_funs *fstream, uint64_t elem) {
    return buffer_get_str(fstream->shmbuffer, elem);
}

struct vms_event_record *vms_funs_stream_get_event_spec(vms_stream_funs *stream,
                                                    vms_kind kind) {
    for (size_t i = 0; i < stream->spec_count; ++i)
        if (stream->events[i].kind == kind)
            return stream->events + i;
    return NULL;
}
