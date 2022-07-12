#include <stdio.h>
#include <unistd.h>

#include "event.h"
#include "stream.h"
#include "shmbuf/buffer.h"

typedef struct _shm_event_regex {
	shm_event base;
    /* the event arguments */
    unsigned char args[];
} shm_event_regex;

typedef struct _shm_stream_sregex {
	shm_stream base;
    struct buffer *shmbuffer;
} shm_stream_sregex;

shm_stream *shm_create_sregex_stream(const char *key);
void shm_destroy_sregex_stream(shm_stream_sregex *ss);

