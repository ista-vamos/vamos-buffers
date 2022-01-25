#define _GNU_SOURCE
#include <stddef.h>
#include <threads.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shm_monitored.h"
//#ifdef LINUX
#include <sys/types.h>
#include <sys/file.h>
#include <sys/mman.h>
//TODO: handle windows case
//#endif

typedef struct databuffer
{
	size_t size_in_pages;
	size_t free_space;
	void* start;
	size_t offset;
	size_t last_offset;
	buffer_entry* last_entry;
	uint64_t id;
	int fd;
} databuffer;

typedef struct buffer
{
	buffer_entry *buf_start;
	buffer_entry *buf_pos;
	buffer_entry *buf_last;
	buffer_entry_id current_id;
	buffer_kind kind;
	int fd;
	size_t size_in_pages;
	databuffer dbuf;
	size_t databuffer_alloc_size_in_pages;
	uint64_t dbufid;
} buffer;


static buffer appbuf;
static thread_local buffer threadbuf;

static void* get_databuffer_space(buffer * buffer, size_t size);

static char *shm_mapname_thread(char *buf)
{
	return shm_mapname_thread_pid_tid(buf, getpid(), gettid());
}
static char *shm_mapname_app(char *buf)
{
	return shm_mapname_thread_pid(buf, getpid());
}
static char *shm_mapname_data(char *buf, uint64_t dbufid)
{
	return shm_mapname_thread_data(buf, getpid(), gettid(), dbufid);
}

static int shamon_shm_open_app_buffer(int flags, mode_t mode)
{
	char name[SHM_NAME_MAXLEN];
	if (shm_mapname_app(name) == 0)
	{
		abort();
	}
	printf("%s\n",name);
	fflush(stdout);
	return open(name, flags | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK | O_CREAT, mode);
}

static int shamon_shm_open_thread_buffer(int flags, mode_t mode)
{
	char name[SHM_NAME_MAXLEN];
	if (shm_mapname_thread(name) == 0)
	{
		abort();
	}
	return open(name, flags | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK | O_CREAT , mode);
}
static int shamon_shm_open_data_buffer(int flags, mode_t mode, uint64_t id)
{
	char name[SHM_NAME_MAXLEN];
	if (shm_mapname_data(name, id) == 0)
	{
		abort();
	}
	return open(name, flags | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK | O_CREAT , mode);
}

void initialize_buffer(buffer *buf, int fd, size_t size_in_pages, buffer_kind kind)
{
	size_t alloc_size = size_in_pages * SHM_PAGE_SIZE;

	void *mem = mmap(0, alloc_size,
					 PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(ftruncate(fd, alloc_size))
	{
		abort();
	}

	buf->buf_start = (buffer_entry *)mem;
	buf->buf_pos = (buffer_entry *)mem;
	buf->current_id = 1;
	buf->kind = kind;
	buf->size_in_pages = size_in_pages;
	buf->databuffer_alloc_size_in_pages=16;
	buf->dbufid=1;
	buf->dbuf.free_space=0;
	buf->dbuf.offset=0;
	buf->dbuf.last_offset=0;
	buf->dbuf.size_in_pages=0;

	size_t entry_count = alloc_size / sizeof(buffer_entry);
	buf->buf_last = buf->buf_start + (entry_count - 1);

	for (buffer_entry *current = buf->buf_start; current < buf->buf_last; current++)
	{
		atomic_init(&current->id, BUF_ENTRY_FREE_ID);
	}
	atomic_init(&buf->buf_last->id, BUF_ENTRY_MAGIC_ID);
}

static buffer_entry *buf_get_prev(buffer* buf)
{
	if(buf->buf_pos==buf->buf_start)
	{
		return buf->buf_last-1;
	}
	return buf->buf_pos-1;
}

void push_wait_for_monitor(buffer *buf, buffer_entry **curpos_ptr, buffer_entry_id *curentryid_ptr)
{
	buffer_entry *curpos = *curpos_ptr;
	buffer_entry_id curentryid = *curentryid_ptr;
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
	*curentryid_ptr = curentryid;
	*curpos_ptr = curpos;
}
void buf_push_event_wait_64(buffer *buf, buffer_entry_kind kind, int64_t payload)
{
	buffer_entry *curpos = buf->buf_pos;
	buffer_entry_id curentryid = atomic_load_explicit(&curpos->id, memory_order_acquire);
	push_wait_for_monitor(buf, &curpos, &curentryid);
	curpos->flags=TBMGMT_STD;
	curpos->kind = kind;
	curpos->payload64_1 = payload;
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 2;
	curpos++;
	buf->buf_pos = curpos;
}
void buf_push_event_nowait_64(buffer *buf, buffer_entry_kind kind, int64_t payload)
{
	buffer_entry *curpos = buf->buf_pos;
	if (curpos == buf->buf_last)
	{
		curpos = buf->buf_start;
	}
	curpos->flags=TBMGMT_STD;
	curpos->kind = kind;
	curpos->payload64_1 = payload;
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 2;
	curpos++;
	buf->buf_pos = curpos;
}
void buf_push_event_wait_32(buffer *buf, buffer_entry_kind kind, int32_t payload)
{
	buffer_entry *curpos = buf->buf_pos;
	buffer_entry_id curentryid = atomic_load_explicit(&curpos->id, memory_order_acquire);
	push_wait_for_monitor(buf, &curpos, &curentryid);
	curpos->flags=TBMGMT_STD;
	curpos->kind = kind;
	curpos->payload32_1 = payload;
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 2;
	curpos++;
	buf->buf_pos = curpos;
}
void buf_push_event_nowait_32(buffer *buf, buffer_entry_kind kind, int32_t payload)
{
	buffer_entry *curpos = buf->buf_pos;
	if (curpos == buf->buf_last)
	{
		curpos = buf->buf_start;
	}
	curpos->kind = kind;
	curpos->payload32_1 = payload;
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 2;
	curpos++;
	buf->buf_pos = curpos;
}
void buf_push_event_wait_32_64(buffer *buf, buffer_entry_kind kind, int32_t payload32, int64_t payload64)
{
	buffer_entry *curpos = buf->buf_pos;
	buffer_entry_id curentryid = atomic_load_explicit(&curpos->id, memory_order_acquire);
	push_wait_for_monitor(buf, &curpos, &curentryid);
	curpos->flags=TBMGMT_STD;
	curpos->kind = kind;
	curpos->payload32_1 = payload32;
	curpos->payload64_1 = payload64;
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 2;
	curpos++;
	buf->buf_pos = curpos;
}
void buf_push_event_nowait_32_64(buffer *buf, buffer_entry_kind kind, int32_t payload32, int64_t payload64)
{
	buffer_entry *curpos = buf->buf_pos;
	if (curpos == buf->buf_last)
	{
		curpos = buf->buf_start;
	}
	curpos->flags=TBMGMT_STD;
	curpos->kind = kind;
	curpos->payload32_1 = payload32;
	curpos->payload64_1 = payload64;
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 2;
	curpos++;
	buf->buf_pos = curpos;
}
void buf_push_event_wait_32_64_64(buffer *buf, buffer_entry_kind kind, int32_t payload32, int64_t payload64, int64_t payload64_2)
{
	buffer_entry *curpos = buf->buf_pos;
	buffer_entry_id curentryid = atomic_load_explicit(&curpos->id, memory_order_acquire);
	push_wait_for_monitor(buf, &curpos, &curentryid);
	curpos->flags=TBMGMT_STD;
	curpos->kind = kind;
	curpos->payload32_1 = payload32;
	curpos->payload64_1 = payload64;
	curpos->payload64_2 = payload64_2;
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 2;
	curpos++;
	buf->buf_pos = curpos;
}
void buf_push_event_nowait_32_64_64(buffer *buf, buffer_entry_kind kind, int32_t payload32, int64_t payload64, int64_t payload64_2)
{
	buffer_entry *curpos = buf->buf_pos;
	if (curpos == buf->buf_last)
	{
		curpos = buf->buf_start;
	}
	curpos->flags=TBMGMT_STD;
	curpos->kind = kind;
	curpos->payload32_1 = payload32;
	curpos->payload64_1 = payload64;
	curpos->payload64_2 = payload64_2;
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 2;
	curpos++;
	buf->buf_pos = curpos;
}
void buf_push_data_wait(buffer* buf, buffer_entry_kind kind, void* data, size_t size)
{
	buffer_entry *curpos = buf->buf_pos;
	void* target = get_databuffer_space(buf, size);
	memcpy(target, data, size);
	buffer_entry_id curentryid = atomic_load_explicit(&curpos->id, memory_order_acquire);
	push_wait_for_monitor(buf, &curpos, &curentryid);
	curpos->flags=TBMGMT_DATA;
	curpos->kind = kind;
	curpos->payload32_1 = buf->dbuf.last_offset;
	curpos->payload64_1 = buf->dbuf.id;
	curpos->payload64_2 = size;
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 2;
	curpos++;
	buf->buf_pos = curpos;
}
void buf_push_data_nowait(buffer *buf, buffer_entry_kind kind, void* data, size_t size)
{
	buffer_entry *curpos = buf->buf_pos;
	void* target = get_databuffer_space(buf, size);
	memcpy(target, data, size);
	if (curpos == buf->buf_last)
	{
		curpos = buf->buf_start;
	}
	curpos->flags=TBMGMT_DATA;
	curpos->kind = kind;
	curpos->payload32_1 = buf->dbuf.last_offset;
	curpos->payload64_1 = buf->dbuf.id;
	curpos->payload64_2 = size;
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 2;
	curpos++;
	buf->buf_pos = curpos;
}

void initialize_application_buffer()
{
	int fd = shamon_shm_open_app_buffer(O_RDWR, 0644);
	if (fd < 0)
	{
		perror("shm_open_appbuf");
		return;
	}

	appbuf.fd = fd;
	appbuf.size_in_pages = 1;
	initialize_buffer(&appbuf, fd, 1, 0);
	buf_push_event_wait_32_64(&appbuf, ABMGMT_HELLO, 0, 0);
}
void intialize_thread_buffer(size_t size_in_pages, buffer_kind kind)
{
	int fd = shamon_shm_open_thread_buffer(O_RDWR, 0644);
	if (fd < 0)
	{
		perror("shm_open_threadbuf");
		return;
	}

	threadbuf.fd = fd;
	threadbuf.size_in_pages = size_in_pages;
	initialize_buffer(&threadbuf, fd, size_in_pages, kind);
	buf_push_event_wait_32_64(&appbuf, ABMGMT_NEW, gettid(), size_in_pages);
}
void app_buffer_wait_for_client()
{
	while(atomic_load_explicit(&buf_get_prev(&appbuf)->id,memory_order_acquire)!=BUF_ENTRY_FREE_ID)
	{
		thrd_sleep(&(struct timespec){.tv_nsec = 1000}, NULL);
	}
}

void close_thread_buffer()
{
	munmap(threadbuf.buf_start, threadbuf.size_in_pages * sysconf(_SC_PAGESIZE));
	close(threadbuf.fd);
	
	char name[SHM_NAME_MAXLEN];
	if (shm_mapname_thread(name) == 0)
	{
		abort();
	}
	unlink(name);
}
void close_app_buffer()
{
	buf_push_event_wait_32(&appbuf, ABMGMT_CLOSE, 1);
	munmap(appbuf.buf_start, appbuf.size_in_pages * sysconf(_SC_PAGESIZE));
	close(appbuf.fd);

	char name[SHM_NAME_MAXLEN];
	if (shm_mapname_app(name) == 0)
	{
		abort();
	}
	unlink(name);
}

static void* get_databuffer_space(buffer * buffer, size_t size)
{
	if(size==0)
	{
		return NULL;
	}
	databuffer * dbuf = &buffer->dbuf;
	//printf("getting space for %lu bytes\n", size);
	if(dbuf->free_space<size)
	{
		// if(dbuf->size_in_pages>0)
		// {
		// 	buf_push_event_wait_32_64_64(&appbuf, ABMGMT_CLOSEDATA, gettid(), buffer->dbufid, dbuf->offset);
		// }
		size_t page_size = SHM_PAGE_SIZE;
		if(dbuf->size_in_pages>0)
		{
			munmap(dbuf->start, dbuf->size_in_pages*page_size);
			close(dbuf->fd);
		}
		size_t size_in_pages = buffer->databuffer_alloc_size_in_pages;
		uint64_t bufid = buffer->dbufid++;
		if(size_in_pages* page_size < size)
		{
			size_in_pages = (size/page_size) + 1;
		}
		else if(size_in_pages * page_size > size * buffer->size_in_pages)
		{
			size_in_pages=(size_in_pages/4) | 1;
		}
		buffer->databuffer_alloc_size_in_pages = size_in_pages * 2;
		size_t newsize = page_size * size_in_pages;
		
		int fd = shamon_shm_open_data_buffer(O_RDWR, 0644, bufid);
		if(fd<0)
		{
			abort();
		}
		void *mem = mmap(0, newsize,
						PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if(ftruncate(fd, newsize))
		{
			abort();
		}
		dbuf->free_space=newsize;
		dbuf->size_in_pages=size_in_pages;
		dbuf->id=bufid;
		dbuf->last_entry=NULL;
		dbuf->last_offset=0;
		dbuf->offset=0;
		dbuf->start=mem;
		dbuf->fd=fd;
		buf_push_event_wait_32_64_64(&appbuf, ABMGMT_NEWDATA, gettid(), size_in_pages, bufid);
		//printf("New buffer %lu: %lu pages\n", dbuf->id, dbuf->size_in_pages);
	}
	dbuf->last_offset=dbuf->offset;
	dbuf->offset+=size;
	dbuf->free_space-=size;
	return (void*)(((intptr_t)dbuf->start)+dbuf->last_offset);
}

void push_event_wait_64(buffer_entry_kind kind, int64_t payload)
{
	buf_push_event_wait_64(&threadbuf, kind, payload);
}
void push_event_nowait_64(buffer_entry_kind kind, int64_t payload)
{
	buf_push_event_nowait_64(&threadbuf, kind, payload);
}

void push_event_wait_32(buffer_entry_kind kind, int32_t payload)
{
	buf_push_event_wait_32(&threadbuf, kind, payload);
}
void push_event_nowait_32(buffer_entry_kind kind, int32_t payload)
{
	buf_push_event_nowait_32(&threadbuf, kind, payload);
}

void push_data_wait(buffer_entry_kind kind, void* data, size_t size)
{
	buf_push_data_wait(&threadbuf, kind, data, size);
}
void push_data_nowait(buffer_entry_kind kind, void* data, size_t size)
{
	buf_push_data_nowait(&threadbuf, kind, data, size);
}


ssize_t wrap_push_read(int fd, void* data, size_t size)
{
	ssize_t result = read(fd, data, size);
	buffer * buf=&threadbuf;

	void* target = get_databuffer_space(buf, size + sizeof(size_t) + sizeof(int64_t));
	
	buffer_entry *curpos = buf->buf_pos;
	*((int64_t*)target) = fd;
	*(((int64_t*)target)+1) = result;
	memcpy((void*)(((char*)target)+sizeof(int64_t)+sizeof(size_t)), data, size);
	buffer_entry_id curentryid = atomic_load_explicit(&curpos->id, memory_order_acquire);
	push_wait_for_monitor(buf, &curpos, &curentryid);
	curpos->flags=TBMGMT_DATA;
	curpos->kind = 1;
	curpos->payload32_1 = buf->dbuf.last_offset;
	curpos->payload64_1 = buf->dbuf.id;
	curpos->payload64_2 = size+sizeof(size_t) + sizeof(int64_t);
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 2;
	curpos++;
	buf->buf_pos = curpos;
	return result;
}
ssize_t wrap_push_write(int fd, const void* data, size_t size)
{
	ssize_t result = write(fd, data, size);
	buffer * buf=&threadbuf;

	void* target = get_databuffer_space(buf, result + sizeof(size_t) + sizeof(int64_t));
	
	buffer_entry *curpos = buf->buf_pos;
	*((int64_t*)target) = fd;
	*(((int64_t*)target)+1) = size;
	memcpy((void*)(((char*)target)+sizeof(int64_t)+sizeof(size_t)), data, result);
	buffer_entry_id curentryid = atomic_load_explicit(&curpos->id, memory_order_acquire);
	push_wait_for_monitor(buf, &curpos, &curentryid);
	curpos->flags=TBMGMT_DATA;
	curpos->kind = 1;
	curpos->payload32_1 = buf->dbuf.last_offset;
	curpos->payload64_1 = buf->dbuf.id;
	curpos->payload64_2 = result+sizeof(size_t) + sizeof(int64_t);
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 2;
	curpos++;
	buf->buf_pos = curpos;
	return result;
}

void push_read(int fd, void* data, size_t size, ssize_t result)
{
	if(result<=0)
	{
		return;
	}
	buffer * buf=&threadbuf;
	//printf("pushing read of %li bytes\n", result);

	void* target = get_databuffer_space(buf, result + sizeof(size_t) + sizeof(int64_t));
	
	buffer_entry *curpos = buf->buf_pos;
	*((int64_t*)target) = fd;
	*(((int64_t*)target)+1) = size;
	memcpy((void*)(((char*)target)+sizeof(int64_t)+sizeof(size_t)), data, result);
	buffer_entry_id curentryid = atomic_load_explicit(&curpos->id, memory_order_acquire);
	push_wait_for_monitor(buf, &curpos, &curentryid);
	curpos->flags=TBMGMT_DATA;
	curpos->kind = 2;
	curpos->payload32_1 = buf->dbuf.last_offset;
	curpos->payload64_1 = buf->dbuf.id;
	curpos->payload64_2 = result+sizeof(size_t) + sizeof(int64_t);
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 2;
	curpos++;
	buf->buf_pos = curpos;
}
void push_write(int fd, const void* data, size_t size, ssize_t result)
{
	if(result<=0)
	{
		return;
	}
	buffer * buf=&threadbuf;
	//printf("pushing write of %li bytes\n", result);

	void* target = get_databuffer_space(buf, result + sizeof(size_t) + sizeof(int64_t));
	
	buffer_entry *curpos = buf->buf_pos;
	*((int64_t*)target) = fd;
	*(((int64_t*)target)+1) = size;
	memcpy((void*)(((char*)target)+sizeof(int64_t)+sizeof(size_t)), data, result);
	buffer_entry_id curentryid = atomic_load_explicit(&curpos->id, memory_order_acquire);
	push_wait_for_monitor(buf, &curpos, &curentryid);
	curpos->flags=TBMGMT_DATA;
	curpos->kind = 1;
	curpos->payload32_1 = buf->dbuf.last_offset;
	curpos->payload64_1 = buf->dbuf.id;
	curpos->payload64_2 = result+sizeof(size_t) + sizeof(int64_t);
	atomic_store_explicit(&curpos->id, buf->current_id, memory_order_release);
	buf->current_id += 2;
	curpos++;
	buf->buf_pos = curpos;
}