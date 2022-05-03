#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "stream-funs.h"
#include "buffer.h"
#include "arbiter.h"
#include "drfun/events.h"

bool funs_is_ready(shm_stream *stream) {
    struct buffer *b = ((shm_stream_funs *)stream)->shmbuffer;
    /* buffer must be ready or it may not be ready anymore, but it
     * still has some data that we haven't read */
    return buffer_is_ready(b) || buffer_size(b) > 0;
}

shm_event *funs_publish_event(shm_stream *stream,
                              shm_event *ev) {
    shm_stream_funs *fstream = (shm_stream_funs *)stream;
    uint64_t *p = (uint64_t *) ev;
    shm_event_funcall *fev = fstream->ev_buff;
    assert(*p < fstream->spec_count && "Invalid idx");
    struct call_event_spec *spec = &fstream->events[*p];
    fev->base.id = shm_stream_get_next_id(stream);
    fev->base.kind = spec->kind;
    fev->base.stream = stream;
    fev->signature = (const char *)spec->signature;

    ++p;
    memcpy(fev->args, p, call_event_spec_get_size(spec));
    return (shm_event *)fev;
}

size_t funs_buffer_events(shm_stream *stream,
                          shm_arbiter_buffer *buffer) {
    shm_stream_funs *ss = (shm_stream_funs *) stream;
    assert(shm_arbiter_buffer_elem_size(buffer) == buffer_elem_size(ss->shmbuffer));
    /* Allow program to continue. Set it here, so that the program starts running only
     * when we start buffering events */
    /* TODO: add a new callback that will be called once before starting the main loop
     * where we can do exactly this "signaling" */
    buffer_set_attached(ss->shmbuffer, true);

    void *elems;
    size_t total = 0;
    size_t size = buffer_size(ss->shmbuffer);

    while (size > 0) {
        elems = buffer_read_pointer(ss->shmbuffer, &size);
        assert(size > 0 && "Buffer memref failed");
        shm_arbiter_buffer_push_k(buffer, elems, size);
        /* printf("Buffered %lu events\n", size); */
        total += size;
        if (!buffer_drop_k(ss->shmbuffer, size)) {
            assert(0 && "Buffer drop failed");
        }
        size = buffer_size(ss->shmbuffer);
    }

    return total;
}

shm_stream *shm_create_funs_stream(const char *key) {
    shm_stream_funs *ss = malloc(sizeof *ss);
    struct buffer *shmbuffer = get_shared_buffer(key);
    assert(shmbuffer && "Getting the shm buffer failed");
    size_t elem_size = buffer_elem_size(shmbuffer);
    assert(elem_size > 0);
    shm_stream_init((shm_stream *)ss,
                    elem_size,
                    funs_buffer_events,
                    funs_publish_event,
                    funs_is_ready,
                    "funs-stream");
    ss->shmbuffer = shmbuffer;
    ss->events = get_shared_control_buffer();
    assert(ss->events);
    size_t evs_num = (control_buffer_size(ss->events)/sizeof(struct call_event_spec));
    size_t ev_size, max_size = 0;

    for (size_t i = 0; i < evs_num; ++i) {
        ss->events[i].kind = shm_mk_event_kind(ss->events[i].name,
                                               sizeof(shm_event_funcall) + call_event_spec_get_size(&ss->events[i]),
                                               NULL, NULL);
        ev_size = shm_event_kind_size(ss->events[i].kind);
        if (ev_size > max_size)
            max_size = ev_size;

        printf("[stream-funs] fun: '%s:%s', kind: '%lu'\n",
               ss->events[i].name, ss->events[i].signature, ss->events[i].kind);
    }

    ss->spec_count = evs_num;
    ss->ev_buff = malloc(max_size);
    assert(ss->ev_buff);

    return (shm_stream *) ss;
}

void shm_event_funcall_release(shm_event_funcall *fev) {
    void *p = fev->args;
    for (const char *o = fev->signature; *o; ++o) {
        if (*o == '_') {
            continue;
        }
        if (*o == 'S') {
            buffer_release_str(((shm_stream_funs*)shm_event_stream((shm_event*)fev))->shmbuffer,
                               *(uint64_t*)p);
            p += sizeof(uint64_t);
            continue;
        }

        p += call_event_op_get_size(*o);
    }
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
