#ifndef VAMOS_CLIENT_H
#define VAMOS_CLIENT_H

typedef struct _vms_shm_buffer vms_shm_buffer;

int buffer_wait_for_monitor(vms_shm_buffer *);

#endif
