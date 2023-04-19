#include <stdio.h>
#include <unistd.h>

#include "vamos-buffers/core/event.h"
#include "vamos-buffers/core/stream.h"

#define DRREGEX_ONLY_ARGS

typedef struct _vms_event_drregex {
    vms_event base;
#ifndef DRREGEX_ONLY_ARGS
    bool write; /* true = write, false = read */
    int fd;
    size_t thread;
#endif
    /* the event arguments */
    unsigned char args[];
} vms_event_drregex;

typedef struct _vms_stream_drregex {
    vms_stream base;
    struct buffer *shmbuffer;
} vms_stream_drregex;

vms_stream *vms_create_drregex_stream(const char *key, const char *name);
void vms_destroy_drregex_stream(vms_stream_drregex *ss);
