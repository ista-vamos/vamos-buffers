#include "stream-generic.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "arbiter.h"
#include "buffer.h"
#include "signatures.h"
#include "source.h"

/* A generic stream for events stored in shared memory buffer,
   no filter nor modification of events supported (unless done manually). */

static bool generic_is_ready(shm_stream *stream) {
    struct buffer *b = stream->incoming_events_buffer;
    /* buffer must be ready or it may not be ready anymore, but it
     * still has some data that we haven't read */
    return buffer_is_ready(b) || buffer_size(b) > 0;
}

static void generic_alter(shm_stream *stream, shm_event *in, shm_event *out) {
    memcpy(out, in, stream->event_size);
}

static void generic_destroy(shm_stream *s) {
    release_shared_buffer(s->incoming_events_buffer);
    free(s);
}

shm_stream *shm_create_generic_stream(const char *key, const char *name,
                                      shm_stream_hole_handling *hole_handling) {
    shm_stream_generic *ss = malloc(sizeof *ss);
    struct buffer *shmbuffer = get_shared_buffer(key);
    assert(shmbuffer && "Getting the shm buffer failed");
    size_t elem_size = buffer_elem_size(shmbuffer);
    assert(elem_size > 0);
    shm_stream_init((shm_stream *)ss, shmbuffer, elem_size, generic_is_ready,
                    NULL, generic_alter, generic_destroy, hole_handling,
                    "generic-stream", name);
    ss->shmbuffer = shmbuffer;

    return (shm_stream *)ss;
}
