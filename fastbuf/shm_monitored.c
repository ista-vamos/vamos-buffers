#define _GNU_SOURCE
#include <stddef.h>
#include <threads.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "shm_monitored.h"
#ifdef LINUX
#include <sys/types.h>
#include <sys/file.h>
#include <sys/mman.h>
//TODO: handle windows case
#endif

typedef struct buffer
{
	buffer_entry *buf_start;
	buffer_entry *buf_pos;
	buffer_entry *buf_last;
	buffer_entry_id current_id;
	buffer_kind kind;
	int fd;
	size_t size_in_pages;
} buffer;

static thread_local buffer threadbuf;

static char *shm_mapname_thread(char *buf)
{
	return shm_mapname_thread_pid_tid(buf, getpid(), gettid());
}

static int shamon_shm_open_thread_buffer(int flags, mode_t mode)
{
	char name[SHM_NAME_MAXLEN];
	if (shm_mapname_thread(name) == 0)
	{
		abort();
	}
	return open(name, flags | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK, mode);
}

void initialize_buffer(buffer *buf, int fd, size_t size_in_pages, buffer_kind kind)
{
	size_t alloc_size = size_in_pages * sysconf(_SC_PAGESIZE);

	void *mem = mmap(0, alloc_size,
					 PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	buf->buf_start = (buffer_entry *)mem;
	buf->buf_pos = (buffer_entry *)mem;
	buf->current_id = 1;
	buf->kind = kind;
	buf->size_in_pages = size_in_pages;

	size_t entry_count = alloc_size / sizeof(buffer_entry);
	buf->buf_last = buf->buf_start + (entry_count - 1);

	for (buffer_entry *current = buf->buf_start; current < buf->buf_last; current++)
	{
		atomic_init(&current->id, BUF_ENTRY_FREE_ID);
	}
	atomic_init(&buf->buf_last->id, BUF_ENTRY_MAGIC_ID);
}

void intialize_thread_buffer(size_t size_in_pages, buffer_kind kind)
{
	int fd = shamon_shm_open_thread_buffer(O_RDWR, 0);
	if (fd < 0)
	{
		perror("shm_open");
		return;
	}

	threadbuf.fd = fd;
	threadbuf.size_in_pages = size_in_pages;
	initialize_buffer(&threadbuf, fd, size_in_pages, kind);
}

void close_thread_buffer()
{
	munmap(threadbuf.buf_start, threadbuf.size_in_pages * sysconf(_SC_PAGESIZE));
	close(threadbuf.fd);
}


void push_event_wait_64(buffer_entry_kind kind, int64_t payload)
{
	buffer *buf = &threadbuf;
	buffer_entry *curpos = buf->buf_pos;
	buffer_entry_id curentryid = atomic_load_explicit(&curpos->id, memory_order_acquire);
	while (curentryid != BUF_ENTRY_FREE_ID)
	{
		if (curentryid == BUF_ENTRY_MAGIC_ID)
		{
			curpos = buf->buf_start;
			curentryid = atomic_load_explicit(&curpos->id, memory_order_acquire);
		}
		else
		{
			spin_wait();
			curentryid = atomic_load_explicit(&curpos->id, memory_order_acquire);
			while (curentryid != BUF_ENTRY_FREE_ID)
			{
				thrd_sleep(&(struct timespec){.tv_nsec = 1000}, NULL);
				curentryid = atomic_load_explicit(&curpos->id, memory_order_acquire);
			}
		}
	}
	curpos->kind = kind;
	curpos->payload64_1 = payload;
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 2;
	curpos++;
	buf->buf_pos = curpos;
}
void push_event_nowait_64(buffer_entry_kind kind, int64_t payload)
{
	buffer *buf = &threadbuf;
	buffer_entry *curpos = buf->buf_pos;
	if (curpos == buf->buf_last)
	{
		curpos = buf->buf_start;
	}
	curpos->kind = kind;
	curpos->payload64_1 = payload;
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 1;
	curpos++;
	buf->buf_pos = curpos;
}
