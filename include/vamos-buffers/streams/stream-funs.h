#include <stdio.h>
#include <unistd.h>

#include "vamos-buffers/core/event.h"
#include "vamos-buffers/core/stream.h"

typedef struct _vms_event_funcall {
    vms_event base;
    /* the event arguments */
    char signature[8];
    unsigned char args[];
} vms_event_funcall;

typedef struct _vms_stream_funs {
    vms_stream base;
    struct buffer *shmbuffer;
    struct event_record *events;
    /* buffer for reading and publishing the event */
    vms_event_funcall *ev;
    void *ev_buff;
    size_t spec_count;
} vms_stream_funs;

bool funs_has_event(vms_stream *stream);

vms_event_funcall *funs_get_next_event(vms_stream *stream);

vms_stream *vms_create_funs_stream(const char *key, const char *name);

struct event_record *vms_funs_stream_get_event_spec(vms_stream_funs *stream,
                                                    vms_kind kind);

void vms_event_funcall_release(vms_event_funcall *fev);

const char *vms_stream_funs_get_str(vms_stream_funs *fstream, uint64_t elem);

void vms_destroy_funs_stream(vms_stream_funs *ss);
