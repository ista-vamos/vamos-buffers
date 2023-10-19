#ifndef VAMOS_CLIENT_H
#define VAMOS_CLIENT_H

typedef struct _vms_shm_buffer vms_shm_buffer;

/**
 * @brief vms_shm_buffer_wait_for_reader waits until a reader connects to the
 * vms_shm_buffer
 * @param buff
 * @return negative errno value on error, otherwise 0
 */
int vms_shm_buffer_wait_for_reader(vms_shm_buffer *);

#endif
