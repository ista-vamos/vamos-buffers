#ifndef VAMOS_SHM_DBG_BUFFER_H
#define VAMOS_SHM_DBG_BUFFER_H

/**
 * dbg buffers
 */

typedef struct _vms_shm_dbg_buffer vms_shm_dbg_buffer;
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

#endif
