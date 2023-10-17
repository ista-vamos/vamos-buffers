#ifndef VAMOS_SHM_BUFFER_PRIVATE_H
#define VAMOS_SHM_BUFFER_PRIVATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/file.h>

#include "vamos-buffers/core/event.h"
#include "vamos-buffers/core/list.h"
#include "vamos-buffers/core/spsc_ringbuf.h"
#include "vamos-buffers/core/vector-macro.h"

struct source_control;
struct event_record;
struct buffer;

#define MAX_AUX_BUF_KEY_SIZE 16
#define DROPPED_RANGES_NUM 5

/* FIXME: this may not be always true */
#define PAGE_SIZE 4096

#define HIDE_SYMBOL __attribute__((visibility("hidden")))

struct dropped_range {
    /* the range of autodropped events
    (for garbage collection) */
    vms_eventid begin;
    vms_eventid end;
};

struct buffer_info {
    vms_spsc_ringbuf ringbuf;

    size_t allocated_size;
    size_t capacity;
    size_t elem_size;
    vms_eventid last_processed_id;
    struct dropped_range dropped_ranges[DROPPED_RANGES_NUM];
    size_t dropped_ranges_next;
    _Atomic _Bool dropped_ranges_lock; /* spin lock */
    /* Number of sub-buffers. Sub-buffers are numbered from 1. */
    volatile _Atomic size_t subbuffers_no;
    /* the monitored program exited/destroyed the buffer */
    volatile _Bool destroyed;
    volatile _Bool monitor_attached;
} __attribute__((aligned(CACHELINE_SIZE)));

struct shmbuffer {
    struct buffer_info info;
    /* pointer to the beginning of data */
    unsigned char data[];
};

struct aux_buffer {
    size_t size;
    size_t head;
    size_t idx;
    uint64_t first_event_id;
    uint64_t last_event_id;
    bool reusable;
    unsigned char data[];
};

/* TODO: cache the shared state in local state
   (e.g., elem_size, etc.). Maybe we could also inline
   the vms_spsc_ringbuf so that we can keep local cache */
struct buffer {
    struct shmbuffer *shmbuffer;
    struct source_control *control;
    /* shared memory of auxiliary buffer */
    struct aux_buffer *cur_aux_buff;
    /* the known aux_buffers that might still be needed */
    VEC(aux_buffers, struct aux_buffer *);
    size_t aux_buf_idx;
    vms_list aux_buffers_age;
    /* shm filedescriptor */
    int fd;
    /* shm key */
    char *key;
    /* mode to set to the created SHM file */
    mode_t mode;
    /* The number of the last subbufer */
    _Atomic size_t last_subbufer_no;
};

#define _ringbuf(buff) (&buff->shmbuffer->info.ringbuf)

struct buffer *initialize_shared_buffer(const char *key, mode_t mode,
                                        size_t elem_size, size_t capacity,
                                        struct source_control *control);

struct buffer *get_shared_buffer(const char *key);
struct buffer *try_get_shared_buffer(const char *key, size_t retry);

size_t compute_vms_size(size_t elem_size, size_t capacity);
size_t compute_vms_buffer_size(size_t nondata_size, size_t elem_size,
                               size_t capacity);

/*** LOCAL buffers ***/
struct buffer *initialize_local_buffer(const char *key, size_t elem_size,
                                       size_t capacity,
                                       struct source_control *control);
void release_local_buffer(struct buffer *buff);

/*** CONTROL buffers ***/
struct source_control *get_shared_control_buffer(const char *buff_key);
struct source_control *create_shared_control_buffer(
    const char *buff_key, mode_t mode, const struct source_control *control);
void release_shared_control_buffer(struct source_control *buffer);
void destroy_shared_control_buffer(const char *buffkey,
                                   struct source_control *buffer);

/*** AUX buffers ***/
size_t aux_buffer_free_space(struct aux_buffer *buff);
void aux_buffer_release(struct aux_buffer *buffer);
struct aux_buffer *writer_get_aux_buffer(struct buffer *buff, size_t size);
struct aux_buffer *reader_get_aux_buffer(struct buffer *buff, size_t idx);

void drop_ranges_lock(struct buffer *buff);
void drop_ranges_unlock(struct buffer *buff);

#endif /* VAMOS_SHM_BUFFER_PRIVATE_H */
