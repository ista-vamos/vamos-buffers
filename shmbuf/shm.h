#ifndef SHAMON_SHM_H
#define SHAMON_SHM_H

#include <stddef.h>
#include <sys/file.h>

const char *shamon_shm_default_key(void);
const char *shamon_shm_default_ctrl_key(void);

int shamon_shm_open(const char *key, int flags, mode_t mode);
int shamon_shm_unlink(const char *key);

char *shm_mapname(const char *name, char *buf);

#endif /* SHAMON_SHM_H */
