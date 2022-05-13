#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "stream-regex.h"
#include "buffer.h"
#include "arbiter.h"
#include "signatures.h"
#include "source.h"

bool sregex_is_ready(shm_stream *stream) {
    struct buffer *b = ((shm_stream_sregex *)stream)->shmbuffer;
    /* buffer must be ready or it may not be ready anymore, but it
     * still has some data that we haven't read */
    return buffer_is_ready(b) || buffer_size(b) > 0;
}

void sregex_alter(shm_stream *stream,
                shm_event *in,
                shm_event *out) {
    memcpy(out, in, stream->event_size);
}

shm_stream *shm_create_sregex_stream(const char *key,
                                     struct source_control **control) {
    shm_stream_sregex *ss = malloc(sizeof *ss);
    struct buffer *shmbuffer = get_shared_buffer(key);
    assert(shmbuffer && "Getting the shm buffer failed");
    size_t elem_size = buffer_elem_size(shmbuffer);
    assert(elem_size > 0);
    shm_stream_init((shm_stream *)ss,
                    shmbuffer,
                    elem_size,
                    sregex_is_ready,
                    NULL,
                    sregex_alter,
                    "regex-stream");
    ss->shmbuffer = shmbuffer;

    void *cntrl = get_shared_control_buffer(key);
    *control = cntrl;
    ss->base.control = cntrl;
    size_t evs_num = ((ss->base.control->size - sizeof(struct source_control))/sizeof(struct event_record));
    size_t ev_size, max_size = 0;

    struct event_record *events = ss->base.control->events;
    for (size_t i = 0; i < evs_num; ++i) {
        ev_size = events[i].size;
        events[i].kind = shm_mk_event_kind(events[i].name,
                                           ev_size,
                                           (const char *)events[i].signature);
        if (ev_size > max_size)
            max_size = ev_size;

        printf("[stream-regex] event '%s', kind: '%lu', size: '%lu', signature: '%s'\n",
               events[i].name, events[i].kind,
               events[i].size, events[i].signature);
    }

    //ss->spec_count = evs_num;

    buffer_set_attached(ss->shmbuffer, true);
    return (shm_stream *) ss;
}

void shm_destroy_sregex_stream(shm_stream_sregex *ss) {
    release_shared_buffer(ss->shmbuffer);
    free(ss);
}

