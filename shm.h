#ifndef SHAMON_SHM_H
#define SHAMON_SHM_H

#include <stddef.h>

struct buffer;

struct buffer *get_shared_buffer(void);
struct buffer *initialize_shared_buffer(void);
void release_shared_buffer(struct buffer *);
/* get a local memory to which we can copy the shared buffer */
struct buffer *get_local_buffer(struct buffer *shared_buffer);

unsigned char *buffer_get_beginning(struct buffer *);
unsigned short buffer_get_monitors_num(struct buffer *);
size_t buffer_get_size(struct buffer *);

/* TODO: move to monitor.h */
void buffer_register_monitor(struct buffer *);
void buffer_register_sync_monitor(struct buffer *);
void buffer_unregister_monitor(struct buffer *);
void buffer_unregister_sync_monitor(struct buffer *);

int buffer_write(struct buffer *buff, void *mem, size_t size);
/* sync shared buffer into a local buffer */
size_t buffer_sync(struct buffer *, struct buffer *);
/* sync shared buffer into a local buffer but always put the new data at
 * the beginning of the buffer (and return the size of data) */
size_t buffer_read(struct buffer *, struct buffer *);
/*
int buffer_sync_read(struct buffer *, unsigned char *pos, void *mem, size_t size);
int buffer_read(struct buffer *, unsigned char *pos, void *mem, size_t size);
*/

#endif /* SHAMON_SHM_H */
