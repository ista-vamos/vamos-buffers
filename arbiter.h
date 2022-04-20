#ifndef SHAMON_ARBITER_H_
#define SHAMON_ARBITER_H_

#include "streams.h"

// the ring buffer used by the arbiter
typedef shm_event*(shm_arbiter_process_events_fn)(shm_arbiter_buffer **buffers,
                                                  size_t buffers_num,
                                                  void *monitor_data)
typedef struct _shm_arbiter {
    shm_streams *streams;
    // one stream has one buffer
    shm_arbiter_buffer *buffers;
} shm_arbiter;

shm_arbiter_create(shm_streams *streams)

#endif // SHAMON_ARBITER_H_
