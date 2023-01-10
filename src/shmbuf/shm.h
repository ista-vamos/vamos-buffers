#ifndef SHAMON_SHM_H
#define SHAMON_SHM_H

#include <stddef.h>
#include <sys/file.h>

#define SHM_NAME_MAXLEN 256

const char *shamon_shm_default_key(void);
const char *shamon_shm_default_ctrl_key(void);

int shamon_shm_open(const char *key, int flags, mode_t mode);
int shamon_shm_rename(const char *old_key, const char *new_key);
int shamon_shm_unlink(const char *key);

char *shm_mapname(const char *name, char *buf);
int shamon_get_tmp_key(const char *key, char *buf, size_t bufsize);
char *shamon_map_ctrl_key(const char *key, char name[SHM_NAME_MAXLEN]);

#endif /* SHAMON_SHM_H */
