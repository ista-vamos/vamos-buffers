#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "shamon.h"
#include "stream.h"
#include "vector.h"

/*****
 * STREAMS
 *****/
typedef struct _shamon {
        shm_vector streams;
} shamon;

shamon *shamon_create(void) {
        shamon *shmn = malloc(sizeof(shamon));
        assert(shmn);

        shm_vector_init(&shmn->streams, sizeof(shm_stream *));
        return shmn;
}

void shamon_destroy(shamon *shmn) {
        shm_vector_destroy(&shmn->streams);
        free(shmn);
}

shm_event *shamon_get_next_ev(shamon *shmn) {
    // use static counter to do round robin -- so that some stream
    // does not starve
    static unsigned i = 0;
    shm_stream *stream = NULL;
    // reset counter if we're at the end
    if (i >= shm_vector_size(&shmn->streams))
        i = 0;

    while (i < shm_vector_size(&shmn->streams)) {
        stream = *((shm_stream**)shm_vector_at(&shmn->streams, i));
        ++i;

        if (stream && stream->has_event(stream)) {
            return stream->get_next_event(stream);
        }
    }

    // TODO: we should check if the stream is finished and remove it
    // in that case
    return NULL;
}

void shamon_add_stream(shamon *shmn, shm_stream *stream) {
    shm_vector_push(&shmn->streams, &stream);
    assert((*((shm_stream**)shm_vector_at(&shmn->streams, shm_vector_size(&shmn->streams) - 1)) == stream)
           && "BUG: shm_vector_push");
    printf("Added a stream id %lu: '%s'\n",
           shm_vector_size(&shmn->streams) - 1,
           stream->name);
}


