#include <stdio.h>
#include <unistd.h>

#include "event.h"
#include "stream.h"
#include "buffer.h"

#define DRREGEX_ONLY_ARGS

typedef struct _shm_event_drregex {
    shm_event base;
#ifndef DRREGEX_ONLY_ARGS
    bool write; /* true = write, false = read */
    int fd;
    size_t thread;
#endif
    /* the event arguments */
    unsigned char args[];
} shm_event_drregex;

typedef struct _shm_stream_drregex {
    shm_stream base;
    struct buffer *shmbuffer;
} shm_stream_drregex;

shm_stream *shm_create_drregex_stream(const char *key,
                                     struct source_control **control);
void shm_destroy_drregex_stream(shm_stream_drregex *ss);

