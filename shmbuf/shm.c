#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "shm.h"

#define SHM_NAME_MAXLEN 256

const char *default_key = "/monitor.shamon.1";
const char *shm_dir = "/dev/shm/";
const size_t shm_dirlen = 9;

const char *shamon_shm_default_key() {
    return default_key;
}

/* adapted function from musl project, src/mman/shm_open.c
 * and from libc sysdeps/posix/shm-directory.h */
char *shm_mapname(const char *name, char *buf)
{
	assert(name[0] == '/');
	/* Construct the filename.  */
	while (name[0] == '/')
		++name;	
	size_t namelen = strlen (name) + 1;
	/* Validate the filename.  */
	if (namelen == 1 || namelen >= SHM_NAME_MAXLEN ||
	    strchr(name, '/') != NULL) {
		assert(0 && "valid shm name");
		return 0;
	}
	if (SHM_NAME_MAXLEN <= shm_dirlen + namelen) {
		assert(0 && "buffer too short");
		return 0;
	}
	memcpy(buf, shm_dir, shm_dirlen);
	memcpy(buf + shm_dirlen, name, namelen);
	buf[shm_dirlen + namelen] = '\0';

	return buf;
}

int shamon_shm_open(const char *key, int flags, mode_t mode)
{
	char name[SHM_NAME_MAXLEN];
	if (shm_mapname(key, name) == 0)
		abort();
	return open(name, flags | O_NOFOLLOW|O_CLOEXEC|O_NONBLOCK, mode);
}

int shamon_shm_unlink(const char *key) {
    if (key == NULL) {
        key = default_key;
    }
	char name[SHM_NAME_MAXLEN];
	if (shm_mapname(key, name) == 0)
		abort();
	return unlink(key);
}
