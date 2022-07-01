#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "utils.h"
#include "stream.h"
#include "arbiter.h"
#include "parallel_queue.h"

#define DROP_SPACE_THRESHOLD 1

typedef struct _shm_arbiter_buffer {
    shm_stream *stream;     // the source for the buffer
    shm_par_queue buffer;   // the buffer itself
    size_t dropped_num;     // the number of dropped events
    size_t total_dropped_times;     // the number of dropped events
    size_t total_dropped_num;     // the number of dropped events
#ifdef DUMP_STATS
    size_t volunt_dropped_num;     // the number of events dropped via drop() calls
    size_t volunt_dropped_num_asked;     // the number of events attempted to
                                         // be dropped dropped via drop() calls
    size_t written_num;     // the number of calls to write_finish
    int last_was_drop;     // true if the last event written was drop()
#endif
    shm_eventid drop_begin_id; // the id of the next 'dropped' event
    bool active;            // true while the events are being queued
} shm_arbiter_buffer;

size_t shm_arbiter_buffer_sizeof(void) {
    return sizeof(shm_arbiter_buffer);
}

void *shm_arbiter_buffer_write_ptr(shm_arbiter_buffer *q) {
    return shm_par_queue_write_ptr(&q->buffer);
}

void shm_arbiter_buffer_write_finish(shm_arbiter_buffer *q) {
#ifdef DUMP_STATS
    ++q->written_num;
#endif
    shm_par_queue_write_finish(&q->buffer);
}

void shm_arbiter_buffer_finish_push(shm_arbiter_buffer *q);

void shm_arbiter_buffer_push_k(shm_arbiter_buffer *q, const void *elems, size_t size);

size_t shm_arbiter_buffer_size(shm_arbiter_buffer *buffer) {
    return shm_par_queue_size(&buffer->buffer);
}

size_t shm_arbiter_buffer_free_space(shm_arbiter_buffer *buffer) {
    return shm_par_queue_free_num(&buffer->buffer);
}

size_t shm_arbiter_buffer_capacity(shm_arbiter_buffer *buffer) {
    return shm_par_queue_capacity(&buffer->buffer);
}

/* drop an event and notify buffer the buffer that it may free up
 * the payload of this and older events */
size_t shm_arbiter_buffer_drop(shm_arbiter_buffer *buffer, size_t k) {
#ifdef DUMP_STATS
    buffer->volunt_dropped_num_asked += k;
#endif
    --k; /* peek_*_at takes index from 0 */
    shm_event *ev = shm_par_queue_peek_atmost_at(&buffer->buffer, &k);
    if (!ev)
        return 0; /* empty queue */
    shm_eventid last_id = shm_event_id(ev);
#ifndef NDEBUG
    size_t n =
#endif
    ++k; /* k is index, we must increase it back by one */
    shm_par_queue_drop(&buffer->buffer, k);
    assert(n == k && "Something changed the queue in between");
    shm_stream_notify_last_processed_id(buffer->stream, last_id);
#ifdef DUMP_STATS
    buffer->volunt_dropped_num += k;
#endif
    return k;
}

bool shm_arbiter_buffer_active(shm_arbiter_buffer *buffer)
{
    return buffer->active;
}

void shm_arbiter_buffer_set_active(shm_arbiter_buffer *buffer, bool val) {
    buffer->active = val;
}

shm_stream *shm_arbiter_buffer_stream(shm_arbiter_buffer *buffer)
{
    return buffer->stream;
}

size_t shm_arbiter_buffer_dropped_num(shm_arbiter_buffer *buffer)
{
    return buffer->total_dropped_num;
}

size_t shm_arbiter_buffer_dropped_times(shm_arbiter_buffer *buffer)
{
    return buffer->total_dropped_times;
}

#ifdef DUMP_STATS
size_t shm_arbiter_buffer_written_num(shm_arbiter_buffer *buffer)
{
    return buffer->written_num;
}
#endif

void shm_arbiter_buffer_init(shm_arbiter_buffer *buffer,
                             shm_stream *stream,
                             size_t out_event_size,
                             size_t capacity) {
    assert(capacity >= 3 && "We need at least 3 elements in the buffer");
    size_t event_size = out_event_size > stream->event_size ? out_event_size : stream->event_size;
    /* the buffer must be able to contain the event 'dropped' */
    if (event_size < sizeof(shm_event_dropped))
        event_size = sizeof(shm_event_dropped);

    shm_par_queue_init(&buffer->buffer, capacity,
                       event_size);

    buffer->stream = stream;
    buffer->active = false;
    buffer->dropped_num = 0;
    buffer->total_dropped_times = 0;
    buffer->total_dropped_num = 0;
#ifdef DUMP_STATS
    buffer->written_num = 0;
    buffer->volunt_dropped_num = 0;
    buffer->volunt_dropped_num_asked = 0;
    buffer->last_was_drop = 0;
#endif
}

shm_arbiter_buffer *shm_arbiter_buffer_create(shm_stream *stream,
                                              size_t out_event_size,
                                              size_t capacity) {
    shm_arbiter_buffer *b = malloc(shm_arbiter_buffer_sizeof());
    assert(b && "Malloc failed");
    shm_arbiter_buffer_init(b, stream, out_event_size, capacity);
    return b;
}
void shm_arbiter_buffer_free(shm_arbiter_buffer *buffer) {
    shm_arbiter_buffer_destroy(buffer);
    free(buffer);
}

void shm_arbiter_buffer_destroy(shm_arbiter_buffer *buffer) {
    shm_par_queue_destroy(&buffer->buffer);
}

size_t shm_arbiter_buffer_elem_size(shm_arbiter_buffer *q) {
    return shm_par_queue_elem_size(&q->buffer);
}

void shm_arbiter_buffer_push(shm_arbiter_buffer *buffer, const void *elem, size_t size) {
    assert(shm_arbiter_buffer_active(buffer));
    shm_par_queue *queue = &buffer->buffer;

    if (buffer->dropped_num > 0) {
        if (shm_par_queue_free_num(queue) < 2) {
            ++buffer->dropped_num;
        } else {
            shm_event_dropped dropped;
            shm_stream_get_dropped_event(buffer->stream, &dropped,
                                         buffer->drop_begin_id,
                                         buffer->dropped_num);
            assert(sizeof(dropped) <= shm_par_queue_elem_size(queue));
#ifndef NDEBUG
            bool ret =
#endif
            shm_par_queue_push(queue, &dropped, sizeof(dropped));
#ifdef DUMP_STATS
            ++buffer->written_num;
#endif
            assert(ret && "BUG: queue has not enough free space");
#ifndef NDEBUG
            ret =
#endif
            shm_par_queue_push(&buffer->buffer, elem, size);
#ifdef DUMP_STATS
            ++buffer->written_num;
#endif
            assert(ret && "BUG: queue has not enough free space");
            buffer->total_dropped_num += buffer->dropped_num;
            ++buffer->total_dropped_times;
            buffer->dropped_num = 0;
            /* the end id may not be precise, but we need just the upper bound */
            shm_arbiter_buffer_notify_dropped(buffer, buffer->drop_begin_id,
                                              shm_event_id((shm_event*)elem) - 1);
        }
    } else {
        if (!shm_par_queue_push(&buffer->buffer, elem, size)) {
            buffer->drop_begin_id = shm_event_id((shm_event*)elem);
            ++buffer->dropped_num;
        }
#ifdef DUMP_STATS
        else {
            ++buffer->written_num;
        }
#endif
    }
}

/*
 * Push k events that reside in a continuous memory
 */
void shm_arbiter_buffer_push_k(shm_arbiter_buffer *buffer,
                               const void *elems,
                               size_t k) {
    //printf("Buffering %lu events\n", k);
    assert(shm_arbiter_buffer_active(buffer));
    shm_par_queue *queue = &buffer->buffer;

    if (buffer->dropped_num > 0) {
        if (shm_par_queue_free_num(queue) < 2) {
            //printf("[buffer] dropping %lu events (free space: %lu)\n", k, shm_par_queue_free_num(queue));
            buffer->dropped_num += k;
        } else {
            //printf("[buffer] generating dropped event 'dropped(%lu)'\n", buffer->dropped_num);
            shm_event_dropped dropped;

            shm_stream_get_dropped_event(buffer->stream, &dropped,
                                         buffer->drop_begin_id, buffer->dropped_num);
            assert(sizeof(dropped) <= shm_par_queue_elem_size(queue));
            assert(shm_par_queue_free_num(queue) > 1);
            shm_par_queue_push(queue, &dropped, sizeof(dropped));
#ifdef DUMP_STATS
            ++buffer->written_num;
#endif
            buffer->total_dropped_num += buffer->dropped_num;
            ++buffer->total_dropped_times;

            /* the end id may not be precise, but we need just the upper bound */
            shm_arbiter_buffer_notify_dropped(buffer, buffer->drop_begin_id,
                                              shm_event_id((shm_event*)elems) - 1);

            assert(shm_par_queue_free_num(queue) >= 1);
            buffer->dropped_num = shm_par_queue_push_k(&buffer->buffer, elems, k); 
#ifdef DUMP_STATS
            buffer->written_num += k - buffer->dropped_num;
#endif
        }
    } else {
        buffer->dropped_num = shm_par_queue_push_k(&buffer->buffer, elems, k);
#ifdef DUMP_STATS
            buffer->written_num += k - buffer->dropped_num;
#endif
        if (buffer->dropped_num > 0) {
            shm_event *first_dropped_ev
                    = shm_par_queue_peek_at(&buffer->buffer, k - buffer->dropped_num);
            assert(first_dropped_ev);
            buffer->drop_begin_id = shm_event_id((shm_event*)first_dropped_ev);
        }
    }
}


/* NOTE: does not notify about processing the event, must
 * be done manually once all the work with data is done */
bool shm_arbiter_buffer_pop(shm_arbiter_buffer *buffer, void *elem) {
    return shm_par_queue_pop(&buffer->buffer, elem);
}

shm_event *shm_arbiter_buffer_top(shm_arbiter_buffer *buffer) {
    return shm_par_queue_top(&buffer->buffer);
}

size_t shm_arbiter_buffer_peek(shm_arbiter_buffer *buffer, size_t n,
                               void **data1, size_t *size1,
                               void **data2, size_t *size2) {
    return shm_par_queue_peek(&buffer->buffer, n, data1, size1, data2, size2);
}

size_t shm_arbiter_buffer_peek1(shm_arbiter_buffer *buffer, void **data) {
    return shm_par_queue_peek1(&buffer->buffer, data);
}


#define SLEEP_TIME_NS_INIT 10
#define BUSY_WAIT_TIMES 1000
#define SLEEP_TIME_NS_THRES 1000000

static void *get_event(shm_stream *stream) {
    /* TODO: if there is no filtering and modifications, we can push multiple events forward.
             if there are filtering and modifications, we could have an additional thread
             to handle the load of data if we copy them in chunks */
    size_t num = 1;
    size_t sleep_time = SLEEP_TIME_NS_INIT;
    size_t spinned = 0;
    void *ev;
    while (1) {
        /* wait for the event */
        ev = shm_stream_read_events(stream, &num);
        if (ev) {
            sleep_time = SLEEP_TIME_NS_INIT;
            spinned = 0;
#ifdef DUMP_STATS
            ++stream->read_events;
#endif
            return ev;
        }

        /* before sleeping, try just to busy wait some time */
        if (spinned < BUSY_WAIT_TIMES) {
            ++spinned;
            continue;
        }

        /* no event read, sleep a while */
        if (sleep_time < SLEEP_TIME_NS_THRES) {
            sleep_time *= 10;
        } else {
            /* checking for the readiness is not cheap,
             * so do it only after we haven't read any
             * event for some time */
            if (!shm_stream_is_ready(stream))
                return NULL;
        }
#ifdef DUMP_STATS
        stream->slept_waiting_for_ev += sleep_time;
#endif
        sleep_ns(sleep_time);
    }

    assert(0 && "Unreachable");
}

static void push_dropped_event(shm_stream *stream,
                               shm_arbiter_buffer *buffer,
	 		       size_t notify_id) {
    shm_event_dropped dropped_ev;
    shm_stream_get_dropped_event(stream, &dropped_ev,
                                 notify_id,
                                 buffer->dropped_num);
    shm_par_queue_push(&buffer->buffer, &dropped_ev, sizeof(dropped_ev));
#ifdef DUMP_STATS
    ++buffer->written_num;
#endif
    buffer->total_dropped_num += buffer->dropped_num;
    ++buffer->total_dropped_times;
    shm_arbiter_buffer_notify_dropped(buffer,
                                      buffer->drop_begin_id,
				      notify_id);
    assert(shm_arbiter_buffer_free_space(buffer) > 0);

    /*
    printf("PUSHED DROPPED event { kind = %lu, id = %lu, n = %lu}\n",
            ((shm_event*)&dropped_ev)->kind,
            ((shm_event*)&dropped_ev)->id,
            dropped_ev.n);
    */
}

/* wait for an event on the 'stream' */
void *stream_fetch(shm_stream *stream,
                   shm_arbiter_buffer *buffer) {
    void *ev;
    size_t last_ev_id = 1;
    while (1) {
        ev = get_event(stream);
        if (!ev) {
	    uint64_t sleep_time = SLEEP_TIME_NS_INIT;
            while (buffer->dropped_num > 0) {
                assert(DROP_SPACE_THRESHOLD < shm_arbiter_buffer_capacity(buffer));
                if (shm_arbiter_buffer_free_space(buffer) > DROP_SPACE_THRESHOLD) {
                    /* the end id may not be precise, but we need just the upper bound */
                    push_dropped_event(stream, buffer, last_ev_id - 1);
                    assert(shm_arbiter_buffer_free_space(buffer) > 0);
#ifdef DUMP_STATS
                    buffer->last_was_drop = 1;
#endif
                    buffer->dropped_num = 0;
                } else {
		    sleep_ns(sleep_time);
		    if (sleep_time < SLEEP_TIME_NS_THRES)
			sleep_time *= 10;
                    assert(buffer->dropped_num > 0);
		}
	    }
            assert(!shm_stream_is_ready(stream));
            assert(buffer->dropped_num == 0);
            return NULL; /* stream ended */
	}

        assert(ev && "Dont have event");
        assert(!shm_event_is_dropped((shm_event*)ev) && "Got dropped event");

	last_ev_id = shm_event_id(ev);
        assert(last_ev_id == ++stream->last_event_id &&
               "IDs are inconsistent");
        /*
        printf("FETCH: read event { kind = %lu, id = %lu}\n",
               ((shm_event*)ev)->kind,
               ((shm_event*)ev)->id);
        */
        if (buffer->dropped_num > 0) {
            assert(DROP_SPACE_THRESHOLD < shm_arbiter_buffer_capacity(buffer));
            if (shm_arbiter_buffer_free_space(buffer) > DROP_SPACE_THRESHOLD) {
                /* the end id may not be precise, but we need just the upper bound */
                assert(last_ev_id == buffer->dropped_num + buffer->drop_begin_id
                       && "Drop IDs are wrong");
                push_dropped_event(stream, buffer, last_ev_id - 1);
                buffer->dropped_num = 0;
                assert(shm_arbiter_buffer_free_space(buffer) > 0);

                /*
                printf("FETCH: stopped dropping { kind = %lu, id = %lu}\n",
                       ((shm_event*)ev)->kind,
                       ((shm_event*)ev)->id);
                        */

#ifdef DUMP_STATS
	        ++stream->fetched_events;
#endif
                return ev;
            }

            ++buffer->dropped_num;
            /* notify about dropped events continuously, because it may take
             * long time to generate the dropped event */
            /* FIXME: % is slow... and make it configurable */
            if (buffer->dropped_num % 10000 == 0) {
                shm_arbiter_buffer_notify_dropped(buffer,
                                                  buffer->drop_begin_id,
                                                  last_ev_id - 1);
            }
            /* consume the dropped event */
            shm_stream_consume(stream, 1);
            continue;
        }

        assert(buffer->dropped_num == 0);
        if (shm_arbiter_buffer_free_space(buffer) == 0) {
            buffer->drop_begin_id = last_ev_id;
            assert(buffer->dropped_num == 0);
            ++buffer->dropped_num;
            shm_stream_consume(stream, 1);
            /*
            printf("FETCH: start dropping { kind = %lu, id = %lu}\n",
                   ((shm_event*)ev)->kind,
                   ((shm_event*)ev)->id);
                */
            continue;
        }

#ifdef DUMP_STATS
	++stream->fetched_events;
#endif
        assert(shm_arbiter_buffer_free_space(buffer) > 0);
        assert(buffer->dropped_num == 0);
        return ev;
    }
}

/* FIXME: do not duplicate the code */
void *stream_filter_fetch(shm_stream *stream,
                          shm_arbiter_buffer *buffer,
                          shm_stream_filter_fn filter) {
    void *ev;
    size_t last_ev_id = 1;
    while (1) {
        ev = get_event(stream);
        if (!ev) {
	    uint64_t sleep_time = SLEEP_TIME_NS_INIT;
            while (buffer->dropped_num > 0) {
                assert(DROP_SPACE_THRESHOLD < shm_arbiter_buffer_capacity(buffer));
                if (shm_arbiter_buffer_free_space(buffer) > DROP_SPACE_THRESHOLD) {
                    /* the end id may not be precise, but we need just the upper bound */
                    push_dropped_event(stream, buffer, last_ev_id - 1);
                    assert(shm_arbiter_buffer_free_space(buffer) > 0);
#ifdef DUMP_STATS
                    buffer->last_was_drop = 1;
#endif
                    buffer->dropped_num = 0;
                } else {
		    sleep_ns(sleep_time);
		    if (sleep_time < SLEEP_TIME_NS_THRES)
			sleep_time *= 10;
                    assert(buffer->dropped_num > 0);
		}
	    }
            assert(!shm_stream_is_ready(stream));
            assert(buffer->dropped_num == 0);
            return NULL; /* stream ended */
	}

        assert(ev && "Dont have event");
        assert(!shm_event_is_dropped((shm_event*)ev) && "Got dropped event");

	last_ev_id = shm_event_id(ev);
        assert(last_ev_id == ++stream->last_event_id &&
               "IDs are inconsistent");

        if (filter && !filter(stream, ev)) {
            continue;
        }

        /*
        printf("FETCH: read event { kind = %lu, id = %lu}\n",
               ((shm_event*)ev)->kind,
               ((shm_event*)ev)->id);
        */
        if (buffer->dropped_num > 0) {
            assert(DROP_SPACE_THRESHOLD < shm_arbiter_buffer_capacity(buffer));
            if (shm_arbiter_buffer_free_space(buffer) > DROP_SPACE_THRESHOLD) {
                /* the end id may not be precise, but we need just the upper bound */
                assert(last_ev_id == buffer->dropped_num + buffer->drop_begin_id
                       && "Drop IDs are wrong");
                push_dropped_event(stream, buffer, last_ev_id - 1);
                buffer->dropped_num = 0;
                assert(shm_arbiter_buffer_free_space(buffer) > 0);

                /*
                printf("FETCH: stopped dropping { kind = %lu, id = %lu}\n",
                       ((shm_event*)ev)->kind,
                       ((shm_event*)ev)->id);
                        */

#ifdef DUMP_STATS
	        ++stream->fetched_events;
#endif
                return ev;
            }

            ++buffer->dropped_num;
            /* notify about dropped events continuously, because it may take
             * long time to generate the dropped event */
            /* FIXME: % is slow... and make it configurable */
            if (buffer->dropped_num % 10000 == 0) {
                shm_arbiter_buffer_notify_dropped(buffer,
                                                  buffer->drop_begin_id,
                                                  last_ev_id - 1);
            }
            /* consume the dropped event */
            shm_stream_consume(stream, 1);
            continue;
        }

        assert(buffer->dropped_num == 0);
        if (shm_arbiter_buffer_free_space(buffer) == 0) {
            buffer->drop_begin_id = last_ev_id;
            assert(buffer->dropped_num == 0);
            ++buffer->dropped_num;
            shm_stream_consume(stream, 1);
            /*
            printf("FETCH: start dropping { kind = %lu, id = %lu}\n",
                   ((shm_event*)ev)->kind,
                   ((shm_event*)ev)->id);
                */
            continue;
        }

#ifdef DUMP_STATS
	++stream->fetched_events;
#endif
        assert(shm_arbiter_buffer_free_space(buffer) > 0);
        assert(buffer->dropped_num == 0);
        return ev;
    }
}


void shm_arbiter_buffer_notify_dropped(shm_arbiter_buffer *buffer,
                                       uint64_t begin_id,
                                       uint64_t end_id) {
    shm_stream_notify_dropped(buffer->stream, begin_id, end_id);
}

#ifdef DUMP_STATS

#define COLOR_RED_IF(c) if ((c)) { fprintf(stderr, "\033[31;1m"); }
#define COLOR_RESET fprintf(stderr, "\033[0m");

void shm_arbiter_buffer_dump_stats(shm_arbiter_buffer *buffer) {
    shm_stream *s = buffer->stream;
    fprintf(stderr, "-- Buffer for stream %lu (%s) --\n", s->id, s->name);
    fprintf(stderr, "   Stream read %lu events from SHM\n", s->read_events);
    COLOR_RED_IF(s->read_events != s->consumed_events)
    fprintf(stderr, "   Stream consumed %lu events from SHM\n", s->consumed_events);
    COLOR_RESET
#ifndef NDEBUG
    /* NOTE: this might be specific to source */
    COLOR_RED_IF(s->last_event_id < s->read_events)
    fprintf(stderr, "   Last event ID on the stream was %lu\n", s->last_event_id);
    COLOR_RESET
#endif
    fprintf(stderr, "   stream_fetch() fetched %lu events\n", s->fetched_events);
    fprintf(stderr, "   stream_fetch() totally dropped %lu events in %lu holes\n",
            buffer->total_dropped_num, buffer->total_dropped_times);
    fprintf(stderr, "   Last event was drop: %s\n",
            buffer->last_was_drop ? "true" : "false");
    COLOR_RED_IF(s->fetched_events + buffer->total_dropped_num != s->consumed_events)
    fprintf(stderr, "     (fetched + dropped = %lu events)\n",
            s->fetched_events + buffer->total_dropped_num);
    COLOR_RESET
    fprintf(stderr, "   The buffer was written to %lu times\n", buffer->written_num);
    COLOR_RED_IF(s->fetched_events + buffer->total_dropped_times
                 != buffer->written_num)
    fprintf(stderr, "     (fetch + num of holes = %lu)\n",
            s->fetched_events + buffer->total_dropped_times);
    COLOR_RESET
    COLOR_RED_IF(buffer->volunt_dropped_num != buffer->written_num)
    fprintf(stderr, "   The buffer consumed %lu events via calls to drop()\n",
            buffer->volunt_dropped_num);
    COLOR_RESET
    COLOR_RED_IF(buffer->volunt_dropped_num_asked < buffer->volunt_dropped_num)
    fprintf(stderr, "     (user asked to consume %lu events)\n",
            buffer->volunt_dropped_num_asked);
    COLOR_RESET
    fprintf(stderr, "   The buffer slept waiting for events %lu ns (%lf sec)\n",
            s->slept_waiting_for_ev, s->slept_waiting_for_ev / (double)1000000000);
}
#endif
