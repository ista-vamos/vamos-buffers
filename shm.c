#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#include "shm.h"

#define SLEEP_TIME 1000
#define MEM_SIZE 65536
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
	/* is the buffer full? */
	_Bool full;
	/* shm filedescriptor */
	int fd; 
} __attribute__((aligned(8)));

struct buffer {
	struct buffer_info __attribute__((aligned(8))) info;
	/* pointer to the beginning of data */
	unsigned char *data;
};

static inline unsigned char *buffer_get_data(struct buffer *buff)
{
	return (unsigned char *)buff + sizeof(struct buffer_info);
}

static inline size_t buffer_get_offset(struct buffer *buff)
{
	return buff->info.pos;
}

static inline unsigned char *buffer_get_pos_pointer(struct buffer *buff)
{
	return buffer_get_data(buff) + buff->info.pos;
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

struct buffer *get_shared_buffer(void)
{
	const char *key = default_key;
	int fd = shm_open(key, O_RDWR, 0);
	if(fd < 0) {
		perror("shm_open");
		return NULL;
	}

	void *mem = mmap(0, MEM_SIZE,
			 PROT_READ|PROT_WRITE|PROT_EXEC, MAP_SHARED, fd, 0);
	if (mem == MAP_FAILED) {
		perror("mmap failure");
		if (close(fd) == -1) {
			perror("closing fd after mmap failure");
		}
		if (shm_unlink(key) == -1) {
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
	int fd = shm_open(key, O_RDWR|O_CREAT, S_IRWXU);
	if(fd < 0) {
		perror("shm_open");
		return NULL;
	}

	if((ftruncate(fd, MEM_SIZE)) == -1) {
		perror("ftruncate");
		return NULL;
	}

	void *mem = mmap(0, MEM_SIZE,
			 PROT_READ|PROT_WRITE|PROT_EXEC, MAP_SHARED, fd, 0);
	if (mem == MAP_FAILED) {
		perror("mmap failure");
		if (close(fd) == -1) {
			perror("closing fd after mmap failure");
		}
		if (shm_unlink(key) == -1) {
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
	struct buffer *buff = (struct buffer *)malloc(shared_buff->info.size);
	if (!buff)
		return NULL;
	memset(buff, 0, sizeof(struct buffer_info));
	buff->info.size = shared_buff->info.size;

	return buff;
}



void release_shared_buffer(struct buffer *buff)
{
	int fd = buff->info.fd;
	if (munmap(buff, buff->info.size)) {
		perror("munmap failure");
	}
	if (close(fd) == -1) {
		perror("closing fd after mmap failure");
	}
	if (shm_unlink(default_key) == -1) {
		perror("shm_unlink after mmap failure");
	}
}

int buffer_write(struct buffer *buff, void *mem, size_t size) {
	if (size > MEM_SIZE)
		return -1;
	if (buffer_get_offset(buff) >= MEM_SIZE - size) {
		/* buffer is full */
		if (buff->info.sync_monitors_num > 0) {
			buff->info.monitors_synced = 0;
			buff->info.full = 1;
			/* wait until all synchronous monitors read the buffer */
			while (buff->info.monitors_synced
					!= buff->info.sync_monitors_num) {
				if (usleep(SLEEP_TIME) == -1) {
					perror("usleep error");
				}
			}
			buff->info.full = 0;
		}

		buff->info.pos = 0;
	}

	memcpy(buffer_get_pos_pointer(buff), mem, size);
	buff->info.pos += size;

	return 0;
}

static inline void checked_sleep(void) {
	if (usleep(SLEEP_TIME) == -1) {
		perror("usleep error");
	}
}

static inline size_t _copy_and_sync_buffers(struct buffer *shm_buff, struct buffer *buff,
		    	                  size_t shmlen, size_t len)
{
	size_t size = shmlen - len;
	/* we always write the new data at the beginning of the buffer */
	memcpy(buffer_get_pos_pointer(buff),
	       buffer_get_data(shm_buff) + (shm_buff->info.pos - size),
	       size);
	buff->info.pos += size;
	assert(buffer_get_offset(buff) == shmlen);
	return size;
}


size_t buffer_read(struct buffer *shm_buff, struct buffer *buff)
{
	/* do we have something to read? */
	size_t shmlen;
	size_t len = buffer_get_offset(buff);

	/* wait for data if needed */
	while ((shmlen = buffer_get_offset(shm_buff)) == len) {
		if (shm_buff->info.full) {
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
			assert(shm_buff->info.sync_monitors_num
					> shm_buff->info.monitors_synced);
			// FIXME: we must do this atomically!
			++buff->info.monitors_synced;
			/* keep waiting until the buffer gets rotated */
			while (shm_buff->info.full) {
				checked_sleep();
			}
			/* shared buffer got rotated, so rotate also
			 * the local buffer */
			buff->info.pos = 0;
			return 0;
		}
		assert(!shm_buff->info.full);
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
