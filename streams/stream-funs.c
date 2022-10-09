#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "arbiter.h"
#include "buffer.h"
#include "signatures.h"
#include "sources/drfun/eventspec.h"
#include "stream-funs.h"

bool funs_is_ready(shm_stream *stream) {
    struct buffer *b = ((shm_stream_funs *)stream)->shmbuffer;
    /* buffer must be ready or it may not be ready anymore, but it
     * still has some data that we haven't read */
    return buffer_is_ready(b) || buffer_size(b) > 0;
}

void funs_alter(shm_stream *stream, shm_event *in, shm_event *out) {
    memcpy(out, in, stream->event_size);
}

static void funs_destroy(shm_stream *s) {
    release_shared_buffer(((shm_stream_funs *)s)->shmbuffer);
    free(s);
}

size_t stream_mk_event_kinds(const char *stream_name, struct buffer *shmbuffer,
                             size_t *max_ev_size);

shm_stream *shm_create_funs_stream(const char *key, const char *name) {
    shm_stream_funs *ss = malloc(sizeof *ss);
    struct buffer *shmbuffer = get_shared_buffer(key);
    assert(shmbuffer && "Getting the shm buffer failed");
    size_t elem_size = buffer_elem_size(shmbuffer);
    assert(elem_size > 0);
    shm_stream_init((shm_stream *)ss, shmbuffer, elem_size, funs_is_ready, NULL,
                    funs_alter, funs_destroy,
                    "funs-stream", name);
    ss->shmbuffer = shmbuffer;

    ss->ev_buff = NULL;

    return (shm_stream *)ss;
}

const char *shm_stream_funs_get_str(shm_stream_funs *fstream, uint64_t elem) {
    return buffer_get_str(fstream->shmbuffer, elem);
}

struct event_record *shm_funs_stream_get_event_spec(shm_stream_funs *stream,
                                                    shm_kind kind) {
    for (size_t i = 0; i < stream->spec_count; ++i)
        if (stream->events[i].kind == kind)
            return stream->events + i;
    return NULL;
}
