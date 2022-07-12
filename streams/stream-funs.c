#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "stream-funs.h"
#include "buffer.h"
#include "arbiter.h"
#include "sources/drfun/eventspec.h"
#include "signatures.h"

bool funs_is_ready(shm_stream *stream) {
    struct buffer *b = ((shm_stream_funs *)stream)->shmbuffer;
    /* buffer must be ready or it may not be ready anymore, but it
     * still has some data that we haven't read */
    return buffer_is_ready(b) || buffer_size(b) > 0;
}

void funs_alter(shm_stream *stream,
                shm_event *in,
                shm_event *out) {
    memcpy(out, in, stream->event_size);
}

static void funs_destroy(shm_stream *s) {
    release_shared_buffer(((shm_stream_funs*)s)->shmbuffer);
    free(s);
}

shm_stream *shm_create_funs_stream(const char *key) {
    shm_stream_funs *ss = malloc(sizeof *ss);
    struct buffer *shmbuffer = get_shared_buffer(key);
    assert(shmbuffer && "Getting the shm buffer failed");
    size_t elem_size = buffer_elem_size(shmbuffer);
    assert(elem_size > 0);
    shm_stream_init((shm_stream *)ss,
                    shmbuffer,
                    elem_size,
                    funs_is_ready,
                    NULL,
                    funs_alter,
                    funs_destroy,
                    "funs-stream");
    ss->shmbuffer = shmbuffer;

    size_t evs_num;
    size_t ev_size, max_size = 0;
    struct event_record *events = buffer_get_avail_events(shmbuffer, &evs_num);
    for (size_t i = 0; i < evs_num; ++i) {
        ev_size = events[i].size;
        events[i].kind = shm_mk_event_kind(events[i].name,
                                           ev_size,
                                           (const char *)events[i].signature);
        if (ev_size > max_size)
            max_size = ev_size;

        printf("[stream-funs] event '%s', kind: '%lu', size: '%lu', signature: '%s'\n",
               events[i].name, events[i].kind,
               events[i].size, events[i].signature);
    }

    ss->spec_count = evs_num;
    ss->ev_buff = malloc(max_size);
    assert(ss->ev_buff);

    buffer_set_attached(ss->shmbuffer, true);
    return (shm_stream *) ss;
}

const char *shm_stream_funs_get_str(shm_stream_funs *fstream, uint64_t elem) {
    return buffer_get_str(fstream->shmbuffer, elem);
}

struct event_record *shm_funs_stream_get_event_spec(shm_stream_funs *stream, shm_kind kind) {
    for (size_t i = 0; i < stream->spec_count; ++i)
        if (stream->events[i].kind == kind)
            return stream->events + i;
    return NULL;
}
