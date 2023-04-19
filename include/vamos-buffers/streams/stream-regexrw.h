#include <stdio.h>
#include <unistd.h>

#include "vamos-buffers/core/event.h"
#include "vamos-buffers/shmbuf/buffer.h"
#include "vamos-buffers/core/stream.h"

typedef struct _vms_event_regexrw {
    vms_event base;
    /* the event arguments */
    unsigned char args[];
} vms_event_regexrw;

typedef struct _vms_stream_sregexrw {
    vms_stream base;
    struct buffer *shmbuffer;
} vms_stream_sregexrw;

vms_stream *vms_create_sregexrw_stream(const char *key, const char *name);
void vms_destroy_sregexrw_stream(vms_stream_sregexrw *ss);
