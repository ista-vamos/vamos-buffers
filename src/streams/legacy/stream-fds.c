#include "stream-fds.h"

#include <assert.h>
#include <poll.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "arbiter.h"

static size_t read_events(shm_stream_fds *ss, shm_arbiter_buffer *buffer) {
    size_t read_ev = 0;
    size_t remove_num = 0;
    shm_event_fd_in ev;

    for (unsigned i = 0; i < ss->fds_num; ++i) {
        struct pollfd *pfd = ss->fds + i;
        if (pfd->revents & POLLIN) {
            shm_string *str = ss->fds_buffer + i;
            // FIXME: set O_NONBLOCK and read all the data right now
            // to have the right timestamps?
            ssize_t len = read(pfd->fd, str->data, str->alloc_size);
            if (len == 0) {
                // EOF, remove fd from fds (set POLLHUP so that we know
                // that this fd should be removed)
                fprintf(stderr, "removing fd after EOF\n");
                pfd->revents &= POLLHUP;
            } else if (len < 0) {
                perror("removing fd after failed read");
                // remove this fd
                pfd->revents &= POLLHUP;
            } else {
                assert(len > 0);
                ev.time = clock();
                // ev.base.stream = (shm_stream *) ss;
                ev.base.kind = ss->ev_kind_in;
                ev.base.id = shm_stream_get_next_id((shm_stream *)ss);
                ev.fd = pfd->fd;
                ev.str_ref.size = len;
                ev.str_ref.data = str->data;
                shm_arbiter_buffer_push(buffer, &ev, sizeof(ev));
                ++read_ev;
            }
        }
        if (pfd->revents & POLLHUP) {
            ++remove_num;
        }
    }

    if (remove_num > 0) {
        printf("Removing fds\n");
        // create a new fds
        ss->fds_num -= remove_num;
        struct pollfd *new_fds = malloc(ss->fds_num * sizeof(struct pollfd));
        assert(new_fds);
        // we could just trim the memory, but...
        shm_string *new_fds_buffer = malloc(ss->fds_num * sizeof(shm_string));
        assert(new_fds_buffer);

        unsigned idx = 0;
        for (unsigned i = 0; i < ss->fds_num; ++i) {
            struct pollfd *pfd = ss->fds + i;
            if (pfd->revents & POLLHUP)
                continue;
            new_fds[idx] = *pfd;
            new_fds_buffer[idx] = ss->fds_buffer[i];
        }
        free(ss->fds);
        free(ss->fds_buffer);
        ss->fds = new_fds;
        ss->fds_size = ss->fds_num;
        ss->fds_buffer = new_fds_buffer;
    }
    return read_ev;
}

static size_t fds_buffer_events(shm_stream *stream,
                                shm_arbiter_buffer *buffer) {
    shm_stream_fds *fs = (shm_stream_fds *)stream;

    int ret = poll(fs->fds, fs->fds_num, 0);
    if (ret == -1) {
        perror("poll failed");
        assert(0 && "poll returned -1");
    } else if (ret > 0) {
        return read_events(fs, buffer);
    }
    return 0;
}

static bool fds_is_ready(shm_stream *stream) {
    return ((shm_stream_fds *)stream)->fds_num > 0;
}

shm_stream *shm_create_fds_stream() {
    shm_stream_fds *ss = malloc(sizeof *ss);
    shm_stream_init((shm_stream *)ss, sizeof(shm_event_fd_in),
                    fds_buffer_events, NULL, fds_is_ready, "fds-stream");
    ss->ev_kind_in = shm_mk_event_kind("fd-in", (shm_stream *)ss,
                                       sizeof(shm_event_fd_in), NULL, NULL);
    ss->fds = NULL;
    ss->fds_size = 0;
    ss->fds_num = 0;
    ss->fds_buffer = NULL;
    shm_queue_init(&ss->pending_events, 32, sizeof(shm_event_fd_in));

    return (shm_stream *)ss;
}

void shm_stream_fds_add_fd(shm_stream_fds *stream, int fd) {
    size_t idx = stream->fds_num++;
    if (idx >= stream->fds_size) {
        stream->fds_size += 8;
        stream->fds =
            realloc(stream->fds, sizeof(struct pollfd) * stream->fds_size);
        stream->fds_buffer =
            realloc(stream->fds_buffer, sizeof(shm_string) * stream->fds_size);
    }
    assert(stream->fds_num < stream->fds_size);

    struct pollfd *pfd = &stream->fds[idx];
    pfd->fd = fd;
    pfd->events = POLLIN;
    stream->fds_buffer[idx].data = malloc(1024);
    assert(stream->fds_buffer[idx].data && "Allocation failed");
    stream->fds_buffer[idx].alloc_size = 1024;
    stream->fds_buffer[idx].size = 0;
}

void shm_destroy_fds_stream(shm_stream_fds *ss) {
    for (unsigned i = 0; i < ss->fds_num; ++i) {
        free(ss->fds_buffer[i].data);
    }
    free(ss->fds_buffer);
    free(ss->fds);
    free(ss);
}
