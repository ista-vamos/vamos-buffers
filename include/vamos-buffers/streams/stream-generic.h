#ifndef SHMN_STREAM_GENERIC_H_
#define SHMN_STREAM_GENERIC_H_

#include <stdio.h>
#include <unistd.h>

#include "vamos-buffers/core/event.h"
#include "vamos-buffers/core/stream.h"

struct buffer;

/* A generic stream for events stored in shared memory buffer,
   no filter nor modification of events supported (unless done manually). */

typedef struct _vms_event_generic {
    vms_event base;
    /* the event arguments */
    unsigned char args[];
} vms_event_generic;

typedef struct _vms_stream_generic {
    vms_stream base;
    struct buffer *shmbuffer;
} vms_stream_generic;

vms_stream *vms_create_generic_stream(const char *key, const char *name,
                                      vms_stream_hole_handling *hole_handling);

#endif /* SHMN_STREAM_GENERIC_H_ */
