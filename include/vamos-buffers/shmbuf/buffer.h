#ifndef SHAMON_SHM_BUFFER_H
#define SHAMON_SHM_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/file.h>

struct source_control;
struct event_record;
struct buffer;

struct buffer *create_shared_buffer(const char *key, size_t capacity,
                                    const struct source_control *control);
struct buffer *create_shared_buffer_adv(const char *key, mode_t mode,
                                        size_t elem_size, size_t capacity,
                                        const struct source_control *control);
struct buffer *create_shared_sub_buffer(struct buffer *buffer, size_t capacity,
                                        const struct source_control *control);
size_t buffer_get_sub_buffers_no(struct buffer *buffer);

struct buffer *try_get_shared_buffer(const char *key, size_t retry);
struct buffer *get_shared_buffer(const char *key);
struct event_record *buffer_get_avail_events(struct buffer *, size_t *);

int buffer_get_key_path(struct buffer *, char keypath[], size_t keypathsize);
int buffer_get_ctrl_key_path(struct buffer *, char keypath[],
                             size_t keypathsize);
const char *buffer_get_key(struct buffer *);
char *get_sub_buffer_key(const char *key, size_t idx);

void release_shared_buffer(struct buffer *);
void destroy_shared_buffer(struct buffer *);
void destroy_shared_sub_buffer(struct buffer *);
void release_shared_sub_buffer(struct buffer *);

bool buffer_is_ready(struct buffer *);
bool buffer_monitor_attached(struct buffer *);
void buffer_set_attached(struct buffer *, bool);
void buffer_set_destroyed(struct buffer *buff);

bool buffer_pop(struct buffer *buff, void *dst);
bool buffer_push(struct buffer *buff, const void *elem, size_t size);
void *buffer_get_str(struct buffer *buff, uint64_t elem);

void *buffer_read_pointer(struct buffer *buff, size_t *size);
bool buffer_drop_k(struct buffer *buff, size_t size);
size_t buffer_consume(struct buffer *buff, size_t k);

size_t buffer_size(struct buffer *buff);
size_t buffer_capacity(struct buffer *buff);
size_t buffer_elem_size(struct buffer *buff);

void *buffer_start_push(struct buffer *buff);

void *buffer_partial_push(struct buffer *buff, void *prev_push,
                          const void *elem, size_t size);
/* evid may be ~(0LL), but then there's no garbage collection */
void *buffer_partial_push_str(struct buffer *buff, void *prev_push,
                              uint64_t evid, const char *str);
void *buffer_partial_push_str_n(struct buffer *buff, void *prev_push,
                                uint64_t evid, const char *str, size_t len);
void buffer_finish_push(struct buffer *buff);

struct aux_buff_ptr {
    uint32_t buffer_id;
    uint32_t offset;
} __attribute__((packed, aligned(64)));

void buffer_set_last_processed_id(struct buffer *buff, uint64_t id);
void buffer_notify_dropped(struct buffer *buffer, uint64_t begin_id,
                           uint64_t end_id);

int buffer_register_event(struct buffer *b, const char *name, uint64_t kind);
int buffer_register_events(struct buffer *b, size_t ev_nums, ...);
int buffer_register_all_events(struct buffer *b);

/**
 * dbg buffers
 */

typedef struct _vms_vms_dbg_buffer vms_vms_dbg_buffer;
vms_vms_dbg_buffer *vms_vms_dbg_buffer_create(const char *key, size_t capacity,
                                              uint16_t key_size,
                                              uint16_t element_size);
vms_vms_dbg_buffer *vms_vms_dbg_buffer_get(const char *key);
void vms_vms_dbg_buffer_release(vms_vms_dbg_buffer *);
void vms_vms_dbg_buffer_destroy(vms_vms_dbg_buffer *);

size_t vms_vms_dbg_buffer_size(vms_vms_dbg_buffer *b);
size_t vms_vms_dbg_buffer_capacity(vms_vms_dbg_buffer *b);
size_t vms_vms_dbg_buffer_key_size(vms_vms_dbg_buffer *b);
size_t vms_vms_dbg_buffer_value_size(vms_vms_dbg_buffer *b);
size_t vms_vms_dbg_buffer_rec_size(vms_vms_dbg_buffer *b);
unsigned char *vms_vms_dbg_buffer_data(vms_vms_dbg_buffer *b);

void vms_vms_dbg_buffer_inc_size(vms_vms_dbg_buffer *b, size_t size);
size_t vms_vms_dbg_buffer_version(vms_vms_dbg_buffer *b);
void vms_vms_dbg_buffer_bump_version(vms_vms_dbg_buffer *b);

#endif /* SHAMON_SHM_BUFFER_H */
