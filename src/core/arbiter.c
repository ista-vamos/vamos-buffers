#include "vamos-buffers/core/arbiter.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "vamos-buffers/core/par_queue.h"
#include "vamos-buffers/core/stream.h"
#include "vamos-buffers/core/utils.h"

#define DROP_SPACE_DEFAULT_THRESHOLD 1

typedef struct _vms_arbiter_buffer {
    vms_par_queue buffer;         // the buffer itself
    size_t drop_space_threshold;  // the number of elements to keep free before
                                  // pushing dropped() event
    size_t dropped_num;           // the number of dropped events
    size_t total_dropped_times;   // the number of dropped events
    size_t total_dropped_num;     // the number of dropped events
#ifdef DUMP_STATS
    size_t volunt_dropped_num;  // the number of events dropped via drop() calls
    size_t volunt_dropped_num_asked;  // the number of events attempted to
                                      // be dropped dropped via drop() calls
    size_t written_num;               // the number of calls to write_finish
    int last_was_drop;      // true if the last event written was drop()
    size_t waited_to_push;  // how many times the buffer waited to push
#endif
    vms_eventid drop_begin_id;  // the id of the next 'dropped' event

    vms_stream *stream;  // the source for the buffer
    vms_event *hole_event;
    bool active;  // true while the events are being queued
} vms_arbiter_buffer;

size_t vms_arbiter_buffer_sizeof(void) { return sizeof(vms_arbiter_buffer); }

void *vms_arbiter_buffer_write_ptr(vms_arbiter_buffer *q) {
    return vms_par_queue_write_ptr(&q->buffer);
}

void vms_arbiter_buffer_write_finish(vms_arbiter_buffer *q) {
#ifdef DUMP_STATS
    ++q->written_num;
#endif
    vms_par_queue_write_finish(&q->buffer);
}

void vms_arbiter_buffer_finish_push(vms_arbiter_buffer *q);

size_t vms_arbiter_buffer_size(vms_arbiter_buffer *buffer) {
    return vms_par_queue_size(&buffer->buffer);
}

size_t vms_arbiter_buffer_free_space(vms_arbiter_buffer *buffer) {
    return vms_par_queue_free_num(&buffer->buffer);
}

size_t vms_arbiter_buffer_capacity(vms_arbiter_buffer *buffer) {
    return vms_par_queue_capacity(&buffer->buffer);
}

size_t vms_arbiter_buffer_set_drop_space_threshold(vms_arbiter_buffer *buffer,
                                                   size_t thr) {
    assert(thr >= 1);
    assert(thr < vms_arbiter_buffer_capacity(buffer));

    size_t old = buffer->drop_space_threshold;
    buffer->drop_space_threshold = thr;
    return old;
}

/* drop an event and notify buffer the buffer that it may free up
 * the payload of this and older events */
size_t vms_arbiter_buffer_drop(vms_arbiter_buffer *buffer, size_t k) {
#ifdef DUMP_STATS
    buffer->volunt_dropped_num_asked += k;
#endif
    --k; /* peek_*_at takes index from 0 */
    vms_event *ev = vms_par_queue_peek_atmost_at(&buffer->buffer, &k);
    if (!ev)
        return 0; /* empty queue */
    vms_eventid last_id = vms_event_id(ev);
#ifndef NDEBUG
    size_t n =
#endif
        ++k; /* k is index, we must increase it back by one */
    vms_par_queue_drop(&buffer->buffer, k);
    assert(n == k && "Something changed the queue in between");
    vms_stream_notify_last_processed_id(buffer->stream, last_id);
#ifdef DUMP_STATS
    buffer->volunt_dropped_num += k;
#endif
    return k;
}

/* Drop all events with ID less or equal to the one of ev.
 * Return how many events were dropped */
size_t vms_arbiter_buffer_drop_older_than(vms_arbiter_buffer *buffer,
                                          vms_eventid id) {
    /* we first must find the event in the queue */
    void *ptr1, *ptr2;
    size_t len1, len2;
    const size_t n =
        vms_par_queue_peek(&buffer->buffer, 0, &ptr1, &len1, &ptr2, &len2);
    if (n == 0)
        return 0;

    const size_t elem_size = vms_par_queue_elem_size(&buffer->buffer);
    size_t k; /* the number of events to be dropped */
    size_t bot = 0, top;
    unsigned char *events;

    assert(len1 > 0 && "Underflow");
    top = len1 - 1;
    vms_event *ev = (vms_event *)((unsigned char *)ptr1 + (top)*elem_size);
    /* in which part of the buffer may the event be? */
    if (id <= vms_event_id(ev)) {
        events = ptr1;
    } else if (len2 > 0) {
        events = ptr2;
        top = len2 - 1;
    } else {
        /* the sought event should be in ptr2 part, but it is empty,
         * therefore we should drop the whole first part */
        k = len1;
        /* now consume everything up to the found event */
        if (k > 0) {
            vms_par_queue_drop(&buffer->buffer, k);
            vms_stream_notify_last_processed_id(buffer->stream, id);

#ifdef DUMP_STATS
            buffer->volunt_dropped_num_asked += k;
            buffer->volunt_dropped_num += k;
#endif
        }

        return k;
    }

    /* find the event (or the first event with smaller ID) */

    size_t pivot = (bot + top) / 2;
#ifndef NDEBUG
    size_t steps = 0;
#endif
    while (true) {
        assert(bot <= top);
        /* the search should terminate much earlier then after n steps */
        assert(steps++ < n && "BUG in binary search");

        ev = (vms_event *)(events + pivot * elem_size);
        if (id < vms_event_id(ev)) {
            if (pivot == 0) {
                assert(bot == 0);
                assert(top == 1);
                top = 0;
            } else {
                assert(pivot > 0);
                top = pivot - 1;
            }
        } else {
            if (bot == pivot) {
                assert(top == bot + 1);
                if (vms_event_id(ev) == id) {
                    top = bot;
                } else {
                    bot = top;
                }
            } else {
                bot = pivot;
            }
        }

        assert(bot <= top);
        if (top == bot) {
            ev = (vms_event *)(events + top * elem_size);
            if (id < vms_event_id(ev)) {
                /* the found event is the one to the left */
                if (bot == 0) {
                    /* There is no event to the left in this part of the buffer.
                     * If we are in the ptr1 part of buffer,
                     * it means there are no events to be dropped, if we're in
                     * ptr2, we should drop the whole ptr1 part */
                    k = (events == ptr1) ? 0 : len1;
                } else {
                    k = (events == ptr1) ? top : len1 + top;
                }
            } else {
                /* the found event is the one at index top,
                   so we must drop top + 1 events */
                k = (events == ptr1) ? top + 1 : len1 + top + 1;
            }
            break;
        }

        pivot = (bot + top) / 2;
    }

    assert(k <= n);

    /* now consume everything up to the found event */
    if (k > 0) {
        vms_par_queue_drop(&buffer->buffer, k);
        vms_stream_notify_last_processed_id(buffer->stream, id);

#ifdef DUMP_STATS
        buffer->volunt_dropped_num_asked += k;
        buffer->volunt_dropped_num += k;
#endif
    }

    return k;
}

bool vms_arbiter_buffer_active(vms_arbiter_buffer *buffer) {
    return buffer->active;
}

void vms_arbiter_buffer_set_active(vms_arbiter_buffer *buffer, bool val) {
    buffer->active = val;
    if (val) {
        vms_stream_attach(buffer->stream);
    } else {
        vms_stream_detach(buffer->stream);
    }
}

vms_stream *vms_arbiter_buffer_stream(vms_arbiter_buffer *buffer) {
    return buffer->stream;
}

size_t vms_arbiter_buffer_dropped_num(vms_arbiter_buffer *buffer) {
    return buffer->total_dropped_num;
}

size_t vms_arbiter_buffer_dropped_times(vms_arbiter_buffer *buffer) {
    return buffer->total_dropped_times;
}

#ifdef DUMP_STATS
size_t vms_arbiter_buffer_written_num(vms_arbiter_buffer *buffer) {
    return buffer->written_num;
}
#endif

void vms_arbiter_buffer_init(vms_arbiter_buffer *buffer, vms_stream *stream,
                             size_t out_event_size, size_t capacity) {
    assert(ADDR_IS_CACHE_ALIGNED(buffer) &&
           "The memory for the buffer is missaligned");
    assert(capacity >= 3 && "We need at least 3 elements in the buffer");
    size_t event_size = out_event_size > stream->event_size
                            ? out_event_size
                            : stream->event_size;
    if (stream->hole_handling.hole_event_size > event_size)
        event_size = stream->hole_handling.hole_event_size;
    /* the buffer must be able to contain the event 'dropped' */
    size_t hole_event_size = stream->hole_handling.hole_event_size;
    if (hole_event_size > 0 && event_size < hole_event_size)
        event_size = hole_event_size;

    vms_par_queue_init(&buffer->buffer, capacity, event_size);

    buffer->drop_space_threshold = DROP_SPACE_DEFAULT_THRESHOLD;
    buffer->hole_event = xalloc(stream->hole_handling.hole_event_size);

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
    buffer->waited_to_push = 0;
#endif
}

vms_arbiter_buffer *vms_arbiter_buffer_create(vms_stream *stream,
                                              size_t out_event_size,
                                              size_t capacity) {
    /* some elements are cache-aligned, so we must make sure
     * that the memory is not allocated mis-aligned */
    vms_arbiter_buffer *b =
        xalloc_aligned(vms_arbiter_buffer_sizeof(), CACHELINE_SIZE);
    vms_arbiter_buffer_init(b, stream, out_event_size, capacity);
    return b;
}
void vms_arbiter_buffer_free(vms_arbiter_buffer *buffer) {
    vms_arbiter_buffer_destroy(buffer);
    free(buffer);
}

void vms_arbiter_buffer_destroy(vms_arbiter_buffer *buffer) {
    vms_par_queue_destroy(&buffer->buffer);
    free(buffer->hole_event);
}

size_t vms_arbiter_buffer_elem_size(vms_arbiter_buffer *q) {
    return vms_par_queue_elem_size(&q->buffer);
}

void vms_arbiter_buffer_push(vms_arbiter_buffer *buffer, const void *elem,
                             size_t size) {
    assert(vms_arbiter_buffer_active(buffer));
    while (!vms_par_queue_push(&buffer->buffer, elem, size)) {
#ifdef DUMP_STATS
        ++buffer->waited_to_push;
#endif
        ;
    }
}

#if 0
void vms_arbiter_buffer_push(vms_arbiter_buffer *buffer, const void *elem,
                             size_t size) {
    assert(vms_arbiter_buffer_active(buffer));
    vms_par_queue *queue = &buffer->buffer;

    if (buffer->dropped_num > 0) {
        if (vms_par_queue_free_num(queue) < 2) {
            ++buffer->dropped_num;
        } else {
            vms_stream_prepare_hole_event(buffer->stream, buffer->hole_event,
                                          buffer->drop_begin_id,
                                          buffer->dropped_num);
            assert(buffer->stream->hole_handling.hole_event_size <=
                   vms_par_queue_elem_size(queue));
#ifndef NDEBUG
            bool ret =
#endif
                vms_par_queue_push(
                    queue, buffer->hole_event,
                    buffer->stream->hole_handling.hole_event_size);
#ifdef DUMP_STATS
            ++buffer->written_num;
#endif
            assert(ret && "BUG: queue has not enough free space");
#ifndef NDEBUG
            ret =
#endif
                vms_par_queue_push(&buffer->buffer, elem, size);
#ifdef DUMP_STATS
            ++buffer->written_num;
#endif
            assert(ret && "BUG: queue has not enough free space");
            buffer->total_dropped_num += buffer->dropped_num;
            ++buffer->total_dropped_times;
            buffer->dropped_num = 0;
            /* the end id may not be precise, but we need just the upper bound
             */
            vms_arbiter_buffer_notify_dropped(buffer, buffer->drop_begin_id,
                                              vms_event_id((vms_event *)elem) -
                                                  1);
        }
    } else {
        if (!vms_par_queue_push(&buffer->buffer, elem, size)) {
            buffer->drop_begin_id = vms_event_id((vms_event *)elem);
            ++buffer->dropped_num;
        }
#ifdef DUMP_STATS
        else {
            ++buffer->written_num;
        }
#endif
    }
}
#endif

/* NOTE: does not notify about processing the event, must
 * be done manually once all the work with data is done */
bool vms_arbiter_buffer_pop(vms_arbiter_buffer *buffer, void *elem) {
    return vms_par_queue_pop(&buffer->buffer, elem);
}

vms_event *vms_arbiter_buffer_top(vms_arbiter_buffer *buffer) {
    return vms_par_queue_top(&buffer->buffer);
}

size_t vms_arbiter_buffer_peek(vms_arbiter_buffer *buffer, size_t n,
                               void **data1, size_t *size1, void **data2,
                               size_t *size2) {
    return vms_par_queue_peek(&buffer->buffer, n, data1, size1, data2, size2);
}

size_t vms_arbiter_buffer_peek1(vms_arbiter_buffer *buffer, void **data) {
    return vms_par_queue_peek1(&buffer->buffer, data);
}

/* get an event from the stream, block until there is some and return it
 * or return NULL if the stream ended */
static void *get_event(vms_stream *stream) {
    /* TODO: if there is no filtering and modifications, we can push multiple
       events forward. if there are filtering and modifications, we could have
       an additional thread to handle the load of data if we copy them in chunks
     */
    size_t num = 1;
    size_t sleep_time = SLEEP_TIME_INIT_NS;
    size_t spinned = 0;
    void *ev;
    while (1) {
        /* wait for the event */
        ev = vms_stream_read_events(stream, &num);
        if (ev) {
#ifdef DUMP_STATS
            ++stream->read_events;
#endif
            return ev;
        }

        /* Before sleeping, try just to busy wait some time.
           After that, sleep, but start with short intervals. */
        /* TODO: assign an expected frequency of events to each source
         * (with some reasonable default value) and sleep according
         * to this value */
        if (++spinned > BUSY_WAIT_FOR_EVENTS) {
            sleep_ns(sleep_time);
#ifdef DUMP_STATS
            stream->slept_waiting_for_ev += sleep_time;
#endif
            if (sleep_time < SLEEP_TIME_THRES_NS) {
                sleep_time *= 2;
            } else {
                /* checking for the readiness is not cheap,
                 * so do it only after we haven't read any
                 * event for some time */
                if (!vms_stream_is_ready(stream)) {
                    return NULL;
                }
            }
        }
    }

    assert(0 && "Unreachable");
}

static void push_dropped_event(vms_stream *stream, vms_arbiter_buffer *buffer,
                               size_t notify_id) {
    vms_stream_prepare_hole_event(stream, buffer->hole_event, notify_id,
                                  buffer->dropped_num);
    vms_par_queue_push(&buffer->buffer, buffer->hole_event,
                       stream->hole_handling.hole_event_size);
#ifdef DUMP_STATS
    ++buffer->written_num;
#endif
    buffer->total_dropped_num += buffer->dropped_num;
    ++buffer->total_dropped_times;
    vms_arbiter_buffer_notify_dropped(buffer, buffer->drop_begin_id, notify_id);
    assert(vms_arbiter_buffer_free_space(buffer) > 0);

    /*
    printf("PUSHED DROPPED event { kind = %lu, id = %lu, n = %lu}\n",
            ((vms_event*)&dropped_ev)->kind,
            ((vms_event*)&dropped_ev)->id,
            dropped_ev.n);
    */
}

void *handle_stream_end(vms_stream *stream, vms_arbiter_buffer *buffer,
                        size_t last_ev_id) {
    uint64_t sleep_time = SLEEP_TIME_INIT_NS;
    while (buffer->dropped_num > 0) {
        assert(buffer->drop_space_threshold <
               vms_arbiter_buffer_capacity(buffer));
        if (vms_arbiter_buffer_free_space(buffer) >
            buffer->drop_space_threshold) {
            /* the end id may not be precise, but we need just the upper
             * bound */
            push_dropped_event(stream, buffer, last_ev_id - 1);
            assert(vms_arbiter_buffer_free_space(buffer) > 0);
#ifdef DUMP_STATS
            buffer->last_was_drop = 1;
#endif
            buffer->dropped_num = 0;
        } else {
            sleep_ns(sleep_time);
            /* the stream is at the end, so we can sleep longer */
            if (sleep_time < 10 * SLEEP_TIME_THRES_NS) {
                sleep_time *= 2;
            }
            assert(buffer->dropped_num > 0);
        }
    }
    assert(!vms_stream_is_ready(stream));
    assert(buffer->dropped_num == 0);
    return NULL; /* stream ended */
}

static inline void send_dropped_event(vms_stream *stream,
                                      vms_arbiter_buffer *buffer,
                                      size_t last_ev_id) {
    assert(last_ev_id == buffer->dropped_num + buffer->drop_begin_id &&
           "Drop IDs are wrong");
    push_dropped_event(stream, buffer, last_ev_id - 1);
    buffer->dropped_num = 0;
    /*
    printf("FETCH: stopped dropping { kind = %lu, id = %lu}\n",
           ((vms_event*)ev)->kind,
           ((vms_event*)ev)->id);
    */
    assert(vms_arbiter_buffer_free_space(buffer) > 0);
}

static inline void start_dropping(vms_stream *stream,
                                  vms_arbiter_buffer *buffer,
                                  vms_event *event) {
    buffer->drop_begin_id = vms_event_id(event);
    assert(buffer->dropped_num == 0);
    ++buffer->dropped_num;

    assert(buffer->hole_event);
    stream->hole_handling.init(buffer->hole_event);
    stream->hole_handling.update(buffer->hole_event, event);
    /*
     printf("FETCH: start dropping { kind = %lu, id = %lu}\n",
            ((vms_event*)ev)->kind,
            ((vms_event*)ev)->id);
     */
    vms_stream_consume(stream, 1);
}

static inline void continue_dropping(vms_stream *stream,
                                     vms_arbiter_buffer *buffer,
                                     vms_event *event) {
    ++buffer->dropped_num;
    stream->hole_handling.update(buffer->hole_event, event);

    /* notify about dropped events continuously, because it may take
     * long time to generate the dropped event */
    /* FIXME: % is slow... and make it configurable */
    if (buffer->dropped_num % 10000 == 0) {
        vms_arbiter_buffer_notify_dropped(buffer, buffer->drop_begin_id,
                                          vms_event_id(event) - 1);
    }
    /* consume the dropped event */
    vms_stream_consume(stream, 1);
}

/* return true if the event should be dropped and false
   if the event should be forwarded */
static bool handle_dropping_event(vms_stream *stream,
                                  vms_arbiter_buffer *buffer,
                                  vms_event *event) {
    if (buffer->dropped_num > 0) {
        assert(buffer->drop_space_threshold <
               vms_arbiter_buffer_capacity(buffer));
        if (vms_arbiter_buffer_free_space(buffer) >
            buffer->drop_space_threshold) {
            send_dropped_event(stream, buffer, vms_event_id(event));
            return false; /* forward the current event */
        }

        continue_dropping(stream, buffer, event);
        return true; /* drop the current event */
    }

    assert(buffer->dropped_num == 0);
    if (vms_arbiter_buffer_free_space(buffer) == 0) {
        start_dropping(stream, buffer, event);
        return true;
    }

    assert(vms_arbiter_buffer_free_space(buffer) > 0);
    assert(buffer->dropped_num == 0);
    return false;
}

/* wait for an event on the 'stream' */
void *vms_stream_fetch(vms_stream *stream) {
    void *ev;
    if ((ev = get_event(stream))) {
        assert(ev && "Dont have event");
        assert(!vms_event_is_hole((vms_event *)ev) && "Got dropped event");
        assert(vms_event_id(ev) == ++stream->last_event_id &&
               "IDs are inconsistent");
        /*
           printf("FETCH: read event { kind = %lu, id = %lu}\n",
                  ((vms_event*)ev)->kind,
                  ((vms_event*)ev)->id);
         */
    }

    return ev;
}

/* Wait for an event on the 'stream'. If `buffer` is full, start dropping. */
void *vms_stream_fetch_dropping(vms_stream *stream,
                                vms_arbiter_buffer *buffer) {
    void *ev;
    size_t last_ev_id = 1;
    while (1) {
        ev = get_event(stream);
        if (!ev) {
            return handle_stream_end(stream, buffer, last_ev_id);
        }

        assert(ev && "Dont have event");
        assert(!vms_event_is_hole((vms_event *)ev) && "Got dropped event");

        last_ev_id = vms_event_id(ev);
        assert(last_ev_id == ++stream->last_event_id && "IDs are inconsistent");
        /*
           printf("FETCH: read event { kind = %lu, id = %lu}\n",
                  ((vms_event*)ev)->kind,
                  ((vms_event*)ev)->id);
         */

        if (!handle_dropping_event(stream, buffer, ev)) {
#ifdef DUMP_STATS
            ++stream->fetched_events;
#endif
            return ev;
        }

        /* go to next iteration to try the next event */
    }
}

/* FIXME: do not duplicate the code */
void *vms_stream_fetch_dropping_filter(vms_stream *stream,
                                       vms_arbiter_buffer *buffer,
                                       vms_stream_filter_fn filter) {
    void *ev;
    size_t last_ev_id = 1;
    while (1) {
        ev = get_event(stream);
        if (!ev) {
            return handle_stream_end(stream, buffer, last_ev_id);
        }

        assert(ev && "Dont have event");
        assert(!vms_event_is_hole((vms_event *)ev) && "Got hole event");

        last_ev_id = vms_event_id(ev);
        assert(last_ev_id == ++stream->last_event_id && "IDs are inconsistent");

        if (filter && !filter(stream, ev)) {
            /* consume the filtered event */
            vms_stream_consume(stream, 1);
            continue;
        }

        /*
           printf("FETCH: read event { kind = %lu, id = %lu}\n",
                  ((vms_event*)ev)->kind,
                  ((vms_event*)ev)->id);
         */

        if (!handle_dropping_event(stream, buffer, ev)) {
#ifdef DUMP_STATS
            ++stream->fetched_events;
#endif
            return ev;
        }

        /* go to next iteration to try the next event */
    }
}

bool vms_arbiter_buffer_is_done(vms_arbiter_buffer *buffer) {
    /* XXX: should we rather use a flag that we set to true when stream-fetch
     * knows that the stream is done? */
    return (buffer->dropped_num == 0 &&
            vms_par_queue_size(&buffer->buffer) == 0) &&
           !vms_stream_is_ready(buffer->stream) &&
           /* there is a race when the buffer may be empty, but after checking
            * the first part of the condition above (resulting in true) a new
            * event is put there. Then the SHM buffer gets destroyed and the
            * rest of the condition above is evaluated to true. In such a case,
            * we could return that the buffer is done event when there is an
            * event in it. So re-check once more that the buffer is really empty
            * after the stream was destroyed */
           (buffer->dropped_num == 0 &&
            vms_par_queue_size(&buffer->buffer) == 0);
}

void vms_arbiter_buffer_notify_dropped(vms_arbiter_buffer *buffer,
                                       uint64_t begin_id, uint64_t end_id) {
    vms_stream_notify_dropped(buffer->stream, begin_id, end_id);
}

#ifdef DUMP_STATS

#define COLOR_RED_IF(c)                \
    if ((c)) {                         \
        fprintf(stderr, "\033[31;1m"); \
    }
#define COLOR_RESET fprintf(stderr, "\033[0m");

void vms_arbiter_buffer_dump_stats(vms_arbiter_buffer *buffer) {
    vms_stream *s = buffer->stream;
    fprintf(stderr, "-- Buffer for stream %lu (%s) --\n", s->id, s->name);
    fprintf(stderr, "   Stream read %lu events from SHM\n", s->read_events);
    COLOR_RED_IF(s->read_events != s->consumed_events)
    fprintf(stderr, "   Stream consumed %lu events from SHM\n",
            s->consumed_events);
    COLOR_RESET
#ifndef NDEBUG
    /* NOTE: this might be specific to source */
    COLOR_RED_IF(s->last_event_id < s->read_events)
    fprintf(stderr, "   Last event ID on the stream was %lu\n",
            s->last_event_id);
    COLOR_RESET
#endif
    fprintf(stderr, "   vms_stream_fetch*() fetched %lu events\n",
            s->fetched_events);
    fprintf(stderr,
            "   vms_stream_fetch*() totally dropped %lu events in %lu holes\n",
            buffer->total_dropped_num, buffer->total_dropped_times);
    fprintf(stderr, "   Last event was drop: %s\n",
            buffer->last_was_drop ? "true" : "false");
    COLOR_RED_IF(s->fetched_events + buffer->total_dropped_num !=
                 s->consumed_events)
    fprintf(stderr, "     (fetched + dropped = %lu events)\n",
            s->fetched_events + buffer->total_dropped_num);
    COLOR_RESET
    fprintf(stderr, "   The buffer was written to %lu times\n",
            buffer->written_num);
    COLOR_RED_IF(s->fetched_events + buffer->total_dropped_times !=
                 buffer->written_num)
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
            s->slept_waiting_for_ev,
            s->slept_waiting_for_ev / (double)1000000000);
}
#endif
