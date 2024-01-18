#ifndef VAMOS_STREAMS_H_
#define VAMOS_STREAMS_H_

#include "vamos-buffers/core/stream.h"

vms_stream *vms_stream_create_from_argv(
    const char *stream_name, int argc, char *argv[],
    const vms_stream_hole_handling *hole_handling);
vms_stream *vms_stream_create(const char *stream_name, const char *spec,
                              const vms_stream_hole_handling *hole_handling);

#endif /* VAMOS_STREAMS_H_ */
