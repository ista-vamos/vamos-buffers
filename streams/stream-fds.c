#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <poll.h>

#include "stream-fds.h"

static void read_events(shm_stream_fds *ss) {
    size_t remove_num = 0;
    for (unsigned i = 0; i < ss->fds_num; ++i) {
        struct pollfd *pfd = ss->fds + i;
        if (pfd->revents & POLLIN) {
            shm_string *str = ss->fds_buffer + i;
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
                 // FIXME: set O_NONBLOCK and read all the data right now
                 // to have the right timestamps?
                 printf("Read from %d:\n'%.*s'\n", pfd->fd, len, str->data);
                //ev.base.size = sizeof(ev);
                //ev.base.stream = stream;
                //ev.base.kind = ss->ev_kind;
                //ev.base.timestamp_lb = (shm_timestamp)clock();
                //ev.base.timestamp_ub = (shm_timestamp)clock();
                //ev.base.id = shm_stream_get_next_id(stream);
                //ev.fd = fileno(fds);
                //ev.str_ref.size = str->size;
                //ev.str_ref.data = str->data;
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

}

static bool fds_has_event(shm_stream *stream) {
    shm_stream_fds *fs = (shm_stream_fds *) stream;
    int ret = poll(fs->fds, fs->fds_num, 0);
    if (ret == -1) {
        perror("poll failed");
        assert(0 && "poll returned -1");
    } else if (ret > 0) {
	    read_events(fs);
	    return true;
    }

    return false;
}

static shm_event_fd_in *fds_get_next_event(shm_stream *stream) {
    static shm_event_fd_in ev_in;
    shm_stream_fds *ss = (shm_stream_fds *) stream;

    return 0;//&ev_in;
}

shm_stream *shm_create_fds_stream() {
    shm_stream_fds *ss = malloc(sizeof *ss);
    shm_stream_init((shm_stream *)ss,
                    (shm_stream_has_event_fn) fds_has_event,
                    (shm_stream_get_next_event_fn) fds_get_next_event,
                     "fds-stream");
    ss->ev_kind_in = shm_mk_event_kind("fd-in");
    ss->fds = NULL;
    ss->fds_size = 0;
    ss->fds_num = 0;
    ss->fds_buffer = NULL;
    return (shm_stream *) ss;
}

void shm_stream_fds_add_fd(shm_stream_fds *stream, int fd) {
    size_t idx = stream->fds_num++;
    if (idx >= stream->fds_size) {
        stream->fds_size += 8;
        stream->fds = realloc(stream->fds, sizeof(struct pollfd) * stream->fds_size);
        stream->fds_buffer = realloc(stream->fds_buffer, sizeof(shm_string) * stream->fds_size);
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
