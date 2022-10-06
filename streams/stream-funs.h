#include <stdio.h>
#include <unistd.h>

#include "event.h"
#include "shmbuf/buffer.h"
#include "stream.h"

typedef struct _shm_event_funcall {
    shm_event base;
    /* the event arguments */
    char signature[8];
    unsigned char args[];
} shm_event_funcall;

typedef struct _shm_stream_funs {
    shm_stream base;
    struct buffer *shmbuffer;
    struct event_record *events;
    /* buffer for reading and publishing the event */
    shm_event_funcall *ev;
    void *ev_buff;
    size_t spec_count;
} shm_stream_funs;

bool funs_has_event(shm_stream *stream);

shm_event_funcall *funs_get_next_event(shm_stream *stream);

shm_stream *shm_create_funs_stream(const char *key, const char *name);

struct event_record *shm_funs_stream_get_event_spec(shm_stream_funs *stream,
                                                    shm_kind kind);

void shm_event_funcall_release(shm_event_funcall *fev);

const char *shm_stream_funs_get_str(shm_stream_funs *fstream, uint64_t elem);

void shm_destroy_funs_stream(shm_stream_funs *ss);
