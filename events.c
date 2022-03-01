#include "eventinterface.h"
#include <stdio.h>
#include <assert.h>

/*****
 * EVENT
 *****/

shm_kind shm_mk_event_kind(char* name) {
    (void)name;
    abort();
};

shm_eventid shm_event_id(shm_event *event) {
	return event->id;
}

shm_timestamp shm_event_timestamp_lb(shm_event *event) {
	return event->timestamp_lb;
};

shm_timestamp shm_event_timestamp_ub(shm_event *event) {
	return event->timestamp_ub;
};

shm_kind shm_event_kind(shm_event *event) {
	return event->kind;
};

size_t shm_event_size(shm_event *event) {
    return event->size;
};


/*****
 * STREAM
 *****/
shm_eventid shm_stream_get_next_id(shm_stream *stream) {
      return ++stream->last_event_id;
}

static uint64_t last_stream_id = 0;

void shm_stream_init(shm_stream *stream,
                     shm_stream_has_event_fn has_event,
                     shm_stream_get_next_event_fn get_next_event,
                     const char * const name) {
        stream->id = ++last_stream_id;
        stream->get_next_event = get_next_event;
        stream->has_event = has_event;
        stream->last_event_id = 0;
        stream->name = name;
}

// just a comfy fun
shm_event *shm_stream_get_next_ev(shm_stream *stream) {
        return stream->get_next_event(stream);
}

/*****
 * STREAMS
 *****/
struct _shm_streams {
        size_t num_of_streams;
        size_t allocated_streams;
        shm_stream **streams;
};

struct _shm_streams global_stream;
shm_streams *shm_streams_mgr(void) {
    return &global_stream;
}

// just a comfy fun
shm_event *shm_streams_get_next_ev(shm_streams *streams_mgr) {
    static unsigned i = 0;
    shm_stream *stream = NULL;
    if (i >= streams_mgr->num_of_streams)
        i = 0;

    while (i < streams_mgr->num_of_streams) {
        assert(streams_mgr->streams);
        stream = streams_mgr->streams[i];
        printf("Dispatching stream %d (%s)\n", i, stream->name);
        if (stream && stream->has_event(stream)) {
            return stream->get_next_event(stream);
        }
        ++i;
    }

    // for now, we should check if the stream is finished...
    return NULL;
}

void shm_streams_add_stream(shm_streams *streams_mgr, shm_stream *stream) {
    size_t num = ++streams_mgr->num_of_streams;

    if (num > streams_mgr->allocated_streams) {
        streams_mgr->allocated_streams += 10;
        streams_mgr->streams = realloc(streams_mgr->streams,
                                       streams_mgr->allocated_streams * sizeof(shm_stream *));
        assert(streams_mgr->streams != NULL);
    }

    assert(num <= streams_mgr->allocated_streams);
    streams_mgr->streams[num - 1] = stream;

}

/*
shm_value shm_event_get_value(shm_event *event, shm_kind field) {
}
*/
