#ifndef VAMOS_SHM_BUFFER_H
#define VAMOS_SHM_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/file.h>

struct vms_source_control;
struct vms_event_record;
typedef struct _vms_shm_buffer vms_shm_buffer;

vms_shm_buffer *create_shared_buffer(const char *key, size_t capacity,
                                     const struct vms_source_control *control);
vms_shm_buffer *create_shared_buffer_adv(
    const char *key, mode_t mode, size_t elem_size, size_t capacity,
    const struct vms_source_control *control);
vms_shm_buffer *create_shared_sub_buffer(
    vms_shm_buffer *buffer, size_t capacity,
    const struct vms_source_control *control);
size_t buffer_get_sub_buffers_no(vms_shm_buffer *buffer);

vms_shm_buffer *try_get_shared_buffer(const char *key, size_t retry);
vms_shm_buffer *get_shared_buffer(const char *key);
struct vms_event_record *buffer_get_avail_events(vms_shm_buffer *, size_t *);

int buffer_get_key_path(vms_shm_buffer *, char keypath[], size_t keypathsize);
int buffer_get_ctrl_key_path(vms_shm_buffer *, char keypath[],
                             size_t keypathsize);
const char *buffer_get_key(vms_shm_buffer *);
char *get_sub_buffer_key(const char *key, size_t idx);

void release_shared_buffer(vms_shm_buffer *);
void destroy_shared_buffer(vms_shm_buffer *);
void destroy_shared_sub_buffer(vms_shm_buffer *);
void release_shared_sub_buffer(vms_shm_buffer *);

bool buffer_is_ready(vms_shm_buffer *);
bool buffer_monitor_attached(vms_shm_buffer *);
void buffer_set_attached(vms_shm_buffer *, bool);
void buffer_set_destroyed(vms_shm_buffer *buff);

bool buffer_pop(vms_shm_buffer *buff, void *dst);
bool buffer_push(vms_shm_buffer *buff, const void *elem, size_t size);
void *buffer_get_str(vms_shm_buffer *buff, uint64_t elem);

void *buffer_read_pointer(vms_shm_buffer *buff, size_t *size);
bool buffer_drop_k(vms_shm_buffer *buff, size_t size);
size_t buffer_consume(vms_shm_buffer *buff, size_t k);

size_t buffer_size(vms_shm_buffer *buff);
size_t buffer_capacity(vms_shm_buffer *buff);
size_t buffer_elem_size(vms_shm_buffer *buff);

void *buffer_start_push(vms_shm_buffer *buff);

void *buffer_partial_push(vms_shm_buffer *buff, void *prev_push,
                          const void *elem, size_t size);
/* evid may be ~(0LL), but then there's no garbage collection */
void *buffer_partial_push_str(vms_shm_buffer *buff, void *prev_push,
                              uint64_t evid, const char *str);
void *buffer_partial_push_str_n(vms_shm_buffer *buff, void *prev_push,
                                uint64_t evid, const char *str, size_t len);
void buffer_finish_push(vms_shm_buffer *buff);

struct aux_buff_ptr {
    uint32_t buffer_id;
    uint32_t offset;
} __attribute__((packed, aligned(64)));

void buffer_set_last_processed_id(vms_shm_buffer *buff, uint64_t id);
void buffer_notify_dropped(vms_shm_buffer *buffer, uint64_t begin_id,
                           uint64_t end_id);

int buffer_register_event(vms_shm_buffer *b, const char *name, uint64_t kind);
int buffer_register_events(vms_shm_buffer *b, size_t ev_nums, ...);
int buffer_register_all_events(vms_shm_buffer *b);

#endif /* VAMOS_SHM_BUFFER_H */
