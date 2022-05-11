#include <stdio.h>
#include <unistd.h>

#include "event.h"
#include "stream.h"
#include "buffer.h"

typedef struct _shm_event_regex {
	shm_event base;
    size_t line;
    /* the event arguments */
    unsigned char args[];
} shm_event_regex;

typedef struct _shm_stream_sregex {
	shm_stream base;
    struct buffer *shmbuffer;
    /*
    struct call_event_spec *events;
    size_t spec_count;
    */
} shm_stream_sregex;

shm_stream *shm_create_sregex_stream(const char *key,
                                     struct source_control **control);
void shm_destroy_sregex_stream(shm_stream_sregex *ss);

