#ifndef SHAMON_H_
#define SHAMON_H_

#include <stdbool.h>
#include <unistd.h>

typedef struct _shamon shamon;
typedef struct _vms_event vms_event;
typedef struct _vms_stream vms_stream;
typedef struct _vms_vector vms_vector;

typedef vms_event *(*shamon_process_events_fn)(vms_vector *buffers, void *data,
                                               vms_stream **);

shamon *shamon_create(shamon_process_events_fn process_events,
                      void *process_events_data);
void shamon_destroy(shamon *);
bool shamon_is_ready(shamon *);
/* for error handling only... */
void shamon_detach(shamon *shmn);

void shamon_add_stream(shamon *shmn, vms_stream *stream,
                       size_t buffer_capacity);
vms_event *shamon_get_next_ev(shamon *, vms_stream **);
vms_vector *shamon_get_buffers(shamon *);
vms_stream **shamon_get_streams(shamon *, size_t *);

#endif  // SHAMON_H_
