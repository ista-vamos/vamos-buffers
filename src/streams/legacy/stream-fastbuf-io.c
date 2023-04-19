#include "stream-fastbuf-io.h"

#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

#include "fastbuf/vms_monitor.h"

// FIXME: do these local to a stream
static int monitoring_active = 0;
static size_t processed_bytes = 0;

typedef struct msgbuf {
    struct msgbuf *next;
    struct msgbuf *prev;
    char *textbuf;
    size_t offset;
} msgbuf;

static void insert_message(msgbuf *buf, char *text) {
    if (buf->textbuf == NULL) {
        buf->textbuf = text;
        buf->offset = sizeof(size_t) + sizeof(int64_t);
    } else {
        msgbuf *newbuf = (msgbuf *)malloc(sizeof(msgbuf));
        newbuf->textbuf = text;
        newbuf->offset = sizeof(size_t) + sizeof(int64_t);
        newbuf->next = buf;
        newbuf->prev = buf->prev;
        newbuf->next->prev = newbuf;
        newbuf->prev->next = newbuf;
    }
}

int monitoring_thread(void *arg) {
    monitor_buffer buffer = (monitor_buffer)arg;
    buffer_entry buffer_buffer[32];
    msgbuf read_msg;
    msgbuf write_msg;
    read_msg.next = &read_msg;
    write_msg.next = &write_msg;
    read_msg.prev = &read_msg;
    write_msg.prev = &write_msg;
    read_msg.textbuf = NULL;
    write_msg.textbuf = NULL;
    while (monitoring_active) {
#ifdef SHM_DOMONITOR_NOWAIT
        size_t count = copy_events_nowait(buffer, buffer_buffer, 32);
#else
        printf("Monitoring events\n");
        size_t count = copy_events_wait(buffer, buffer_buffer, 32);
#endif
        printf("Recv bytes: %lu\n", count);
        for (size_t i = 0; i < count; i++) {
            if (buffer_buffer[i].kind == 1) {
                char *text = ((char *)(buffer_buffer[i].payload64_1)) +
                             sizeof(size_t) + sizeof(int64_t);
                // insert_message(&write_msg, ((char
                // *)(buffer_buffer[i].payload64_1)) + sizeof(size_t) +
                // sizeof(int64_t));
                printf("%s\n", text);
            } else {
                char *text = ((char *)(buffer_buffer[i].payload64_1)) +
                             sizeof(size_t) + sizeof(int64_t);
                // insert_message(&read_msg, ((char
                // *)(buffer_buffer[i].payload64_1)) + sizeof(size_t) +
                // sizeof(int64_t));
                printf("%s\n", text);
            }
            processed_bytes += buffer_buffer[i].payload64_2;
        }
    }
    return 0;
}

int register_monitored_thread(monitor_buffer buffer) {
    thrd_t thrd;
    printf("Registering thread\n");
    thrd_create(&thrd, &monitoring_thread, buffer);
}

static bool io_has_event(vms_stream *stream) {
    // vms_stream_io *fs = (vms_stream_io *) stream;

    //// dispatch pending events if available
    // if (vms_queue_size(fs->pending_events) > 0)
    //     return true;

    //// check for new events
    // int ret = poll(fs->io, fs->io_num, 0);
    // if (ret == -1) {
    //     perror("poll failed");
    //     assert(0 && "poll returned -1");
    // } else if (ret > 0) {
    //     size_t num = read_events(fs);
    //     // num can be 0 if all io get closed (which
    //     // is what poll detects)
    //     assert(num == 0 || vms_queue_size(fs->pending_events));
    // }

    // return vms_queue_size(fs->pending_events);
    return true;
}

static vms_event_io *io_get_next_event(vms_stream *stream) {
    // vms_stream_io *ss = (vms_stream_io *) stream;
    // static vms_event_fd_in ev;
    // if (vms_queue_pop(ss->pending_events, &ev))
    //     return &ev;

    return NULL;
}

vms_stream *vms_create_io_stream(pid_t pid) {
    char *name = malloc(21);  // FIXME: we leak this
    assert(name);
    snprintf(name, 21, "io-stream(%d)", pid);

    vms_stream_io *ss = malloc(sizeof *ss);
    assert(ss);
    vms_stream_init((vms_stream *)ss, sizeof(vms_event_io),
                    (vms_stream_has_event_fn)io_has_event,
                    (vms_stream_get_next_event_fn)io_get_next_event, name);
    ss->ev_kind_in =
        vms_mk_event_kind("io-in", sizeof(vms_event_io), NULL, NULL);
    ss->ev_kind_out =
        vms_mk_event_kind("io-out", sizeof(vms_event_io), NULL, NULL);

    monitoring_active = 1;
    monitored_process proc = attach_to_process(pid, &register_monitored_thread);

    // wait_for_process(proc);

    return (vms_stream *)ss;
}

void vms_destroy_io_stream(vms_stream_io *s) { monitoring_active = 0; }
