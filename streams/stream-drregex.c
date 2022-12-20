#include "stream-drregex.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "arbiter.h"
#include "buffer.h"
#include "signatures.h"
#include "source.h"

bool drregex_is_ready(shm_stream *stream) {
    struct buffer *b = ((shm_stream_drregex *)stream)->shmbuffer;
    /* buffer must be ready or it may not be ready anymore, but it
     * still has some data that we haven't read */
    return buffer_is_ready(b) || buffer_size(b) > 0;
}

void drregex_alter(shm_stream *stream, shm_event *in, shm_event *out) {
    memcpy(out, in, stream->event_size);
}

void drregex_destroy(shm_stream *s) {
    shm_stream_drregex *ss = (shm_stream_drregex *)s;
    release_shared_buffer(ss->shmbuffer);
    free(ss);
}

shm_stream *shm_create_drregex_stream(const char *key, const char *name) {
    shm_stream_drregex *ss = malloc(sizeof *ss);
    struct buffer *shmbuffer = get_shared_buffer(key);
    assert(shmbuffer && "Getting the shm buffer failed");
    size_t elem_size = buffer_elem_size(shmbuffer);
    assert(elem_size > 0);
    shm_stream_init((shm_stream *)ss, shmbuffer, elem_size, drregex_is_ready,
                    NULL, drregex_alter, drregex_destroy, NULL,
                    "drregex-stream", name);
    ss->shmbuffer = shmbuffer;

    return (shm_stream *)ss;
}
