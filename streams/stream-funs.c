#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "stream-funs.h"
#include "buffer.h"
#include "arbiter.h"
#include "sources/drfun/events.h"

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
                    "funs-stream");
    ss->shmbuffer = shmbuffer;
    ss->events = get_shared_control_buffer();
    assert(ss->events);
    size_t evs_num = (control_buffer_size(ss->events)/sizeof(struct call_event_spec));
    size_t ev_size, max_size = 0;

    for (size_t i = 0; i < evs_num; ++i) {
        assert(ss->events[i].size == 0 ||
               sizeof(shm_event_funcall) + call_event_spec_get_size(&ss->events[i]) <= ss->events[i].size);
        ss->events[i].kind = shm_mk_event_kind(ss->events[i].name,
                                               /*sizeof(shm_event_funcall) + call_event_spec_get_size(&ss->events[i]),*/
                                               ss->events[i].size);
        ev_size = shm_event_size_for_kind(ss->events[i].kind);
        if (ev_size > max_size)
            max_size = ev_size;

        printf("[stream-funs] fun: '%s:%s', kind: '%lu', size: '%lu'\n",
               ss->events[i].name, ss->events[i].signature,
               ss->events[i].kind, ss->events[i].size);
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

void shm_destroy_funs_stream(shm_stream_funs *ss) {
    release_shared_buffer(ss->shmbuffer);
    free(ss);
}

struct call_event_spec *shm_funs_stream_get_event_spec(shm_stream_funs *stream, shm_kind kind) {
    for (size_t i = 0; i < stream->spec_count; ++i)
        if (stream->events[i].kind == kind)
            return stream->events + i;
    return NULL;
}
