#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "arbiter.h"
#include "buffer.h"
#include "signatures.h"
#include "source.h"
#include "stream-regex.h"

bool sregex_is_ready(shm_stream *stream) {
    struct buffer *b = ((shm_stream_sregex *)stream)->shmbuffer;
    /* buffer must be ready or it may not be ready anymore, but it
     * still has some data that we haven't read */
    return buffer_is_ready(b) || buffer_size(b) > 0;
}

void sregex_alter(shm_stream *stream, shm_event *in, shm_event *out) {
    memcpy(out, in, stream->event_size);
}

static void sregex_destroy(shm_stream *s) {
    release_shared_buffer(((shm_stream_sregex *)s)->shmbuffer);
    free(s);
}

shm_stream *shm_create_sregex_stream(const char *key, const char *name) {
    shm_stream_sregex *ss = malloc(sizeof *ss);
    struct buffer *shmbuffer = get_shared_buffer(key);
    assert(shmbuffer && "Getting the shm buffer failed");
    size_t elem_size = buffer_elem_size(shmbuffer);
    assert(elem_size > 0);
    shm_stream_init((shm_stream *)ss, shmbuffer, elem_size, sregex_is_ready,
                    NULL, sregex_alter, sregex_destroy,
                    "regex-stream", name);
    ss->shmbuffer = shmbuffer;

    return (shm_stream *)ss;
}
