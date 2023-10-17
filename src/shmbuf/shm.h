#ifndef VAMOS_SHM_H
#define VAMOS_SHM_H

#include <stddef.h>
#include <sys/file.h>

#define SHM_NAME_MAXLEN 256

const char *shamon_vms_default_key(void);
const char *shamon_vms_default_ctrl_key(void);

int vms_shm_open(const char *key, int flags, mode_t mode);
int vms_shm_rename(const char *old_key, const char *new_key);
int vms_shm_unlink(const char *key);

char *vms_shm_mapname(const char *name, char *buf);
int vms_shm_get_tmp_key(const char *key, char *buf, size_t bufsize);
char *vms_shm_map_ctrl_key(const char *key, char name[SHM_NAME_MAXLEN]);

#endif /* VAMOS_SHM_H */
