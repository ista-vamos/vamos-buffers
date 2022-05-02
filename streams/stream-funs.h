#include <stdio.h>
#include <unistd.h>

#include "event.h"
#include "stream.h"
#include "buffer.h"

typedef struct _shm_event_funcall {
	shm_event base;
    /* the event arguments */
    const char *signature;
    unsigned char args[0];
} shm_event_funcall;

typedef struct _shm_stream_funs {
	shm_stream base;
    struct buffer *shmbuffer;
    struct call_event_spec *events;
    /* buffer for reading and publishing the event */
    shm_event_funcall *ev;
    void *ev_buff;
    size_t spec_count;
} shm_stream_funs;

bool funs_has_event(shm_stream *stream);

shm_event_funcall *funs_get_next_event(shm_stream *stream);

shm_stream *shm_create_funs_stream(const char *key);

struct call_event_spec *shm_funs_stream_get_event_spec(shm_stream_funs *stream, shm_kind kind);

void shm_destroy_funs_stream(shm_stream_funs *ss);

