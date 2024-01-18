#include <stdio.h>
#include <unistd.h>

#include "vamos-buffers/core/event.h"
#include "vamos-buffers/core/stream.h"
#include "vamos-buffers/shmbuf/buffer.h"

typedef struct _vms_event_regex {
    vms_event base;
    /* the event arguments */
    unsigned char args[];
} vms_event_regex;

typedef struct _vms_stream_sregex {
    vms_stream base;
    vms_shm_buffer *shmbuffer;
} vms_stream_sregex;

vms_stream *vms_create_sregex_stream(
    const char *key, const char *name,
    const vms_stream_hole_handling *hole_handling);
void vms_destroy_sregex_stream(vms_stream_sregex *ss);
