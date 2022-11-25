#ifndef SHAMON_H_
#define SHAMON_H_

#include <stdbool.h>
#include <unistd.h>

typedef struct _shamon     shamon;
typedef struct _shm_event  shm_event;
typedef struct _shm_stream shm_stream;
typedef struct _shm_vector shm_vector;

typedef shm_event *(*shamon_process_events_fn)(shm_vector *buffers, void *data,
                                               shm_stream **);

shamon *shamon_create(shamon_process_events_fn process_events,
                      void                    *process_events_data);
void    shamon_destroy(shamon *);
bool    shamon_is_ready(shamon *);
/* for error handling only... */
void shamon_detach(shamon *shmn);

void         shamon_add_stream(shamon *shmn, shm_stream *stream,
                               size_t buffer_capacity);
shm_event   *shamon_get_next_ev(shamon *, shm_stream **);
shm_vector  *shamon_get_buffers(shamon *);
shm_stream **shamon_get_streams(shamon *, size_t *);

#endif // SHAMON_H_
