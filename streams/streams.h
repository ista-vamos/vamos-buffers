#ifndef SHAMON_STREAMS_H_
#define SHAMON_STREAMS_H_

#include "stream.h"

shm_stream *
shm_stream_create_from_argv(const char *stream_name, int argc, char *argv[],
                            const shm_stream_hole_handling *hole_handling);
shm_stream *shm_stream_create(const char *stream_name, const char *spec,
                              const shm_stream_hole_handling *hole_handling);

#endif /* SHAMON_STREAMS_H_ */
