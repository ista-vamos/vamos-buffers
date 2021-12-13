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
#define SLEEP_TIME_NS 10000
#define MEM_SIZE 1041664
const char *default_key = "/monitor.shamon.1";

struct buffer_info {
	/* size of the buffer*/
	size_t size;
	/* next write position */
	size_t pos;
	/* number of monitors monitoring this buffer */
	unsigned short monitors_num;
	/* how many monitors need all events, unused atm */
	unsigned short sync_monitors_num;
	/* counter of already synced monitors, unused atm */
	unsigned short monitors_synced;
	/* is the buffer full? (serves also as a counter) */
	size_t full;
	/* the monitored program exited/destroyed the buffer */
	_Bool destroyed;
	/* shm filedescriptor */
	int fd; 
} __attribute__((aligned(8)));

struct buffer {
	struct buffer_info __attribute__((aligned(8))) info;
	/* pointer to the beginning of data */
	unsigned char data[MEM_SIZE];
};

static inline unsigned char *buffer_get_data(struct buffer *buff)
{
	return buff->data;
}

static inline size_t buffer_get_offset(struct buffer *buff)
{
	return buff->info.pos;
}

static inline unsigned char *buffer_get_pos_pointer(struct buffer *buff)
{
	return buffer_get_data(buff) + buff->info.pos;
}

_Bool buffer_is_ready(struct buffer *buff)
{
	return !buff->info.destroyed;
}

unsigned char *buffer_get_beginning(struct buffer *buff)
{
	return buffer_get_data(buff);
}

unsigned short buffer_get_monitors_num(struct buffer *buff)
{
	return buff->info.monitors_num;
}

size_t buffer_get_size(struct buffer *buff)
{
	return buff->info.size;
}

const char *shm_dir = "/dev/shm/";
const size_t shm_dirlen = 9;

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

static int shamon_shm_open(const char *key, int flags, mode_t mode)
{
	char name[SHM_NAME_MAXLEN];
	if (shm_mapname(key, name) == 0)
		abort();
	return open(name, flags | O_NOFOLLOW|O_CLOEXEC|O_NONBLOCK, mode);
}

static int shamon_shm_unlink(const char *key) {
	char name[SHM_NAME_MAXLEN];
	if (shm_mapname(key, name) == 0)
		abort();
	return unlink(key);
}

struct buffer *get_shared_buffer(void)
{
	const char *key = default_key;
	int fd = shamon_shm_open(key, O_RDWR, 0);
	if(fd < 0) {
		perror("shm_open");
		return NULL;
	}

	void *mem = mmap(0, sizeof(struct buffer),
			 PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (mem == MAP_FAILED) {
		perror("mmap failure");
		if (close(fd) == -1) {
			perror("closing fd after mmap failure");
		}
		if (shamon_shm_unlink(key) != 0) {
			perror("shm_unlink after mmap failure");
		}
		return NULL;
	}

	struct buffer *buff = (struct buffer *)mem;
	/* FIXME -- we need to store fd somewhere locally... */
	assert(buff->info.size == MEM_SIZE);
//	buff->info.fd = fd;

	return buff;
}

struct buffer *initialize_shared_buffer(void)
{
	const char *key = default_key;
	int fd = shamon_shm_open(key, O_RDWR|O_CREAT, S_IRWXU);
	if(fd < 0) {
		perror("shm_open");
		return NULL;
	}

	if((ftruncate(fd, sizeof(struct buffer))) == -1) {
		perror("ftruncate");
		return NULL;
	}

	void *mem = mmap(0, sizeof(struct buffer),
			 PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (mem == MAP_FAILED) {
		perror("mmap failure");
		if (close(fd) == -1) {
			perror("closing fd after mmap failure");
		}
		if (shamon_shm_unlink(key) != 0) {
			perror("shm_unlink after mmap failure");
		}
		return NULL;
	}

	struct buffer *buff = (struct buffer *)mem;
	memset(buff, 0, sizeof(struct buffer_info));
	buff->info.size = MEM_SIZE;
	buff->info.fd = fd;

	return buff;
}

struct buffer *get_local_buffer(struct buffer *shared_buff)
{
	struct buffer *buff = (struct buffer *)malloc(sizeof(struct buffer));
	if (!buff)
		return NULL;
	memset(buff, 0, sizeof(struct buffer_info));
	buff->info.size = shared_buff->info.size;

	return buff;
}


void release_shared_buffer(struct buffer *buff)
{
	int fd = buff->info.fd;
	if (munmap(buff, sizeof(struct buffer)) != 0) {
		perror("munmap failure");
	}
	if (close(fd) == -1) {
		perror("closing fd after mmap failure");
	}
	if (shamon_shm_unlink(default_key) != 0) {
		perror("shm_unlink failure");
	}
}


void destroy_shared_buffer(struct buffer *buff)
{
	buff->info.destroyed = 1;
	release_shared_buffer(buff);
}



static inline void checked_sleep(void) {
	static struct timespec ts = { .tv_nsec = SLEEP_TIME_NS };
	if (nanosleep(&ts, NULL) == -1) {
		perror("nanosleep error");
	}
}

void buffer_rotate(struct buffer *buff) {
	if (buff->info.sync_monitors_num > 0) {
		buff->info.monitors_synced = 0;
		assert(buff->info.monitors_synced == 0);
		/* XXX: we assume only one writer */
		++buff->info.full;
		/* wait until all synchronous monitors read the buffer */
		while (buff->info.monitors_synced
				!= buff->info.sync_monitors_num) {
			checked_sleep();
		}
		++buff->info.full;
	}

	buff->info.pos = 0;
}


int buffer_write(struct buffer *buff, void *mem, size_t size) {
	assert(!buff->info.destroyed && "Writing to a destroyed buffer");

	/*
	if (size > MEM_SIZE)
		return -1;
	*/
	if (buffer_get_offset(buff) > MEM_SIZE - size) {
		/* buffer is full, wait until all monitors
		 * are synced and then rotate it */
		buffer_rotate(buff);
	}

	memcpy(buffer_get_pos_pointer(buff), mem, size);
	buff->info.pos += size;

	return 0;
}

static inline size_t _copy_and_sync_buffers(struct buffer *shm_buff, struct buffer *buff,
					    size_t shmlen, size_t len)
{
	assert (shmlen >= len && "Got negative size");
	size_t size = shmlen - len;
	if (size == 0)
		return 0;
	/* we always write the new data at the beginning of the buffer */
	memcpy(buffer_get_data(buff),
	       buffer_get_data(shm_buff) + shmlen - size,
	       size);
	buff->info.pos += size;
	return size;
}


size_t buffer_read(struct buffer *shm_buff, struct buffer *buff)
{
	/* do we have something to read? */
	size_t shmlen;
	size_t len = buffer_get_offset(buff);
	size_t fullid = buff->info.full;

	/* wait for data if needed */
	while ((shmlen = buffer_get_offset(shm_buff)) == len) {
		if (shm_buff->info.full != fullid) {
			/* there may have been some data written
			 * between the last check and setting the
			 * full flag. Read these data, so that we are
			 * synced in the next call of buffer_read() */
			if (shmlen != len) {
				return _copy_and_sync_buffers(shm_buff, buff,
				                              shmlen, len);
			}

			/* we are synced (as the shared buffer cannot change
			 * when it is full until we signal it */
			assert(shm_buff->info.full == fullid + 1);
			assert(shm_buff->info.sync_monitors_num
					> shm_buff->info.monitors_synced);
			// __sync_fetch_and_add(&shm_buff->info.monitors_synced, 1);
			++shm_buff->info.monitors_synced;
			/* monitors_synced now cannot be reset */
			assert(shm_buff->info.monitors_synced >= 1);

			/* keep waiting until the buffer gets rotated */
			while (shm_buff->info.full == fullid + 1) {
				checked_sleep();
			}
			/* shared buffer got rotated, so rotate also
			 * the local buffer */
			buff->info.pos = 0;
			buff->info.full += 2;
			return _copy_and_sync_buffers(shm_buff, buff,
			                              buffer_get_offset(shm_buff), 0);
		}

		if (shm_buff->info.destroyed) {
			shmlen = buffer_get_offset(shm_buff);
			if (shmlen != len) {
				return _copy_and_sync_buffers(shm_buff, buff,
				                              shmlen, len);
			}
			return 0;
		}
		/* if we copied everything and the buffer got destroyed,
		 * just bail out */
		checked_sleep();
	}

	return _copy_and_sync_buffers(shm_buff, buff, shmlen, len);
}

/* TODO: make these operations atomic once we have multiple monitors/clients */
void buffer_register_monitor(struct buffer *buff) {
	++buff->info.monitors_num;
}

void buffer_register_sync_monitor(struct buffer *buff) {
	++buff->info.sync_monitors_num;
	++buff->info.monitors_num;
	assert(buff->info.sync_monitors_num <= buff->info.monitors_num);
}

void buffer_unregister_monitor(struct buffer *buff) {
	assert(buff->info.monitors_num > 0);
	--buff->info.monitors_num;
}

void buffer_unregister_sync_monitor(struct buffer *buff) {
	assert(buff->info.monitors_num > 0);
	assert(buff->info.sync_monitors_num > 0);
	--buff->info.monitors_num;
	--buff->info.sync_monitors_num;
	assert(buff->info.sync_monitors_num <= buff->info.monitors_num);
}
