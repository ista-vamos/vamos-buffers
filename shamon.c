#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "shamon.h"
#include "stream.h"

/*****
 * STREAMS
 *****/
typedef struct _shamon {
        size_t num_of_streams;
        size_t allocated_streams;
        shm_stream **streams;
} shamon;

shamon *shamon_create(void) {
        shamon *shmn = malloc(sizeof(shamon));
        assert(shmn);

        shmn->num_of_streams = 0;
        shmn->allocated_streams = 0;
        shmn->streams = NULL;

        return shmn;
}

void shamon_destroy(shamon *shmn) {
        free(shmn->streams);
        free(shmn);
}

shm_event *shamon_get_next_ev(shamon *shmn) {
    // use static counter to do round robin -- so that some stream
    // does not starve
    static unsigned i = 0;
    shm_stream *stream = NULL;
    // reset counter if we're at the end
    if (i >= shmn->num_of_streams)
        i = 0;

    while (i < shmn->num_of_streams) {
        assert(shmn->streams);
        stream = shmn->streams[i];
        // printf("Dispatching stream %d (%s)\n", i, stream->name);
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
    size_t num = shmn->num_of_streams++;

    if (num >= shmn->allocated_streams) {
        shmn->allocated_streams += 10;
        shmn->streams = realloc(shmn->streams,
                                       shmn->allocated_streams * sizeof(shm_stream *));
        assert(shmn->streams != NULL);
    }

    assert(num < shmn->allocated_streams);
    shmn->streams[num] = stream;
    printf("Added a stream id %lu: '%s'\n", num, stream->name);
}


