#include "vamos-buffers/streams/stream-generic.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "vamos-buffers/core/arbiter.h"
#include "vamos-buffers/shmbuf/buffer.h"

/* A generic stream for events stored in shared memory buffer,
   no filter nor modification of events supported (unless done manually). */

static bool generic_is_ready(vms_stream *stream) {
    vms_shm_buffer *b = stream->incoming_events_buffer;
    /* buffer must be ready or it may not be ready anymore, but it
     * still has some data that we haven't read */
    return buffer_is_ready(b) || buffer_size(b) > 0;
}

static void generic_alter(vms_stream *stream, vms_event *in, vms_event *out) {
    memcpy(out, in, stream->event_size);
}

vms_stream *vms_create_generic_stream(const char *key, const char *name,
                                      vms_stream_hole_handling *hole_handling) {
    vms_stream_generic *ss = malloc(sizeof *ss);
    vms_shm_buffer *shmbuffer = get_shared_buffer(key);
    assert(shmbuffer && "Getting the shm buffer failed");
    size_t elem_size = buffer_elem_size(shmbuffer);
    assert(elem_size > 0);
    vms_stream_init((vms_stream *)ss, shmbuffer, elem_size, generic_is_ready,
                    NULL, generic_alter, NULL, hole_handling, "generic-stream",
                    name);
    ss->shmbuffer = shmbuffer;

    return (vms_stream *)ss;
}
