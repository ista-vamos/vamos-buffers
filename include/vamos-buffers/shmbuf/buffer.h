#ifndef VAMOS_SHM_BUFFER_H
#define VAMOS_SHM_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/file.h>

struct vms_source_control;
struct vms_event_record;
typedef struct _vms_shm_buffer vms_shm_buffer;

/* ---------------------------------------------------------------------- */
/* WRITER API                                                             */
/* ---------------------------------------------------------------------- */
vms_shm_buffer *vms_shm_buffer_create(const char *key, size_t capacity,
                                      const struct vms_source_control *control);
vms_shm_buffer *vms_shm_buffer_create_adv(
    const char *key, mode_t mode, size_t elem_size, size_t capacity,
    const struct vms_source_control *control);
vms_shm_buffer *vms_shm_buffer_create_sub_buffer(
    vms_shm_buffer *buffer, size_t capacity,
    const struct vms_source_control *control);
size_t vms_shm_buffer_get_sub_buffers_no(vms_shm_buffer *buffer);

void vms_shm_buffer_destroy(vms_shm_buffer *);
void vms_shm_buffer_destroy_sub_buffer(vms_shm_buffer *);

bool vms_shm_buffer_push(vms_shm_buffer *buff, const void *elem, size_t size);
void *vms_shm_buffer_start_push(vms_shm_buffer *buff);

void *vms_shm_buffer_partial_push(vms_shm_buffer *buff, void *prev_push,
                                  const void *elem, size_t size);
/* evid may be ~(0LL), but then there's no garbage collection */
void *vms_shm_buffer_partial_push_str(vms_shm_buffer *buff, void *prev_push,
                                      uint64_t evid, const char *str);
void *vms_shm_buffer_partial_push_str_n(vms_shm_buffer *buff, void *prev_push,
                                        uint64_t evid, const char *str,
                                        size_t len);
void vms_shm_buffer_finish_push(vms_shm_buffer *buff);

/* ---------------------------------------------------------------------- */
/* READER API                                                             */
/* ---------------------------------------------------------------------- */
vms_shm_buffer *vms_shm_buffer_try_connect(const char *key, size_t retry);
vms_shm_buffer *vms_shm_buffer_connect(const char *key);
struct vms_event_record *vms_shm_buffer_get_avail_events(vms_shm_buffer *,
                                                         size_t *);

void vms_shm_buffer_release(vms_shm_buffer *);
void vms_shm_buffer_release_sub_buffer(vms_shm_buffer *);

bool vms_shm_buffer_pop(vms_shm_buffer *buff, void *dst);
void *vms_shm_buffer_read_str(vms_shm_buffer *buff, uint64_t elem);
void *vms_shm_buffer_read_pointer(vms_shm_buffer *buff, size_t *size);
bool vms_shm_buffer_drop_k(vms_shm_buffer *buff, size_t size);
size_t vms_shm_buffer_consume(vms_shm_buffer *buff, size_t k);

int vms_shm_buffer_register_event(vms_shm_buffer *b, const char *name,
                                  uint64_t kind);
int vms_shm_buffer_register_events(vms_shm_buffer *b, size_t ev_nums, ...);
int vms_shm_buffer_register_all_events(vms_shm_buffer *b);

/* ---------------------------------------------------------------------- */
/* GENERIC AND AUXILIARY API                                              */
/* ---------------------------------------------------------------------- */

size_t vms_shm_buffer_size(vms_shm_buffer *buff);
size_t vms_shm_buffer_capacity(vms_shm_buffer *buff);
size_t vms_shm_buffer_elem_size(vms_shm_buffer *buff);

/* default flags */
enum _vms_shm_buffer_flag {
    READER_IS_READY = 1UL << 0,
    READER_FINISHED = 1UL << 1,
    LAST_FLAG = READER_FINISHED
};

void vms_shm_buffer_set_flags(vms_shm_buffer *, uint64_t);
void vms_shm_buffer_unset_flags(vms_shm_buffer *, uint64_t);

/**
 * @brief vms_shm_buffer_reader_is_ready checks if the reader has called
 * vms_shm_buffer_set_reader_is_ready();
 * vms_shm_buffer_reader_is_ready and vms_shm_buffer_reader_is_ready _may_ be
 * used to synchronize the writer and the reader before transmiting data.
 */
bool vms_shm_buffer_reader_is_ready(vms_shm_buffer *);
void vms_shm_buffer_set_reader_is_ready(vms_shm_buffer *);

/* buffer is ready to transmit data (it can be written to if there is space) */
bool vms_shm_buffer_is_ready(vms_shm_buffer *);
/* buffer was not destroyed on the writer's side */
bool vms_shm_buffer_is_destroyed(vms_shm_buffer *);

void vms_shm_buffer_set_destroyed(vms_shm_buffer *buff);

const char *vms_shm_buffer_key(vms_shm_buffer *);

int vms_shm_buffer_compute_key_path(vms_shm_buffer *, char keypath[],
                                    size_t keypathsize);
int vms_shm_buffer_compute_ctrl_key_path(vms_shm_buffer *, char keypath[],
                                         size_t keypathsize);
char *vms_shm_buffer_compute_sub_buffer_key(const char *key, size_t idx);

/* ---------------------------------------------------------------------- */
/* AUXILIARY BUFFERS                                                      */
/* ---------------------------------------------------------------------- */

struct aux_buff_ptr {
    uint32_t buffer_id;
    uint32_t offset;
} __attribute__((packed, aligned(64)));

void vms_shm_buffer_set_last_processed_id(vms_shm_buffer *buff, uint64_t id);
void vms_shm_buffer_notify_dropped(vms_shm_buffer *buffer, uint64_t begin_id,
                                   uint64_t end_id);
#endif /* VAMOS_SHM_BUFFER_H */
