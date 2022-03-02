#define _GNU_SOURCE
#include "shm_monitor.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <threads.h>
#include <fcntl.h>
//#ifdef LINUX
#include <sys/types.h>
#include <sys/file.h>
#include <sys/mman.h>
//TODO: handle windows case
//#endif

typedef struct databuffer
{
	struct databuffer *next;
	struct databuffer *prev;
	uint64_t id;
	void *start;
	size_t size_in_pages;
	size_t last_offset;
	size_t final_offset;
	int fd;
} databuffer;
struct monitor_buffer
{
	buffer_entry *buf_start;
	buffer_entry *buf_pos;
	buffer_entry *buf_prev;
	buffer_entry *buf_last;
	int fd;
	size_t size_in_pages;
	int stopped;
	databuffer *dbuf;
	mtx_t data_mtx;
	cnd_t data_cond;
};

struct monitor_buffer_node
{
	struct monitor_buffer buffer;
	pid_t tid;
	struct monitor_buffer_node *next;
	struct monitor_buffer_node *prev;
};

typedef struct monitored_process
{
	struct monitor_buffer buffer;
	pid_t pid;
	thrd_t main_event_loop_thrd;
	int status;
	mtx_t status_mtx;
	cnd_t status_cond;
	int (*register_thread_buffer)(monitor_buffer buffer);
	struct monitor_buffer_node *thread_buffers;
} * monitored_process;

static int shamon_shm_unlink(monitor_buffer buffer, pid_t pid, pid_t tid)
{
	char name[SHM_NAME_MAXLEN];
	if (shm_mapname_thread_pid_tid(name, pid, tid) == 0)
	{
		abort();
	}
	return unlink(name);
}

void wait_for_process(monitored_process proc)
{
	mtx_lock(&proc->status_mtx);
	while (proc->status == 0)
	{
		cnd_wait(&proc->status_cond, &proc->status_mtx);
	}
	mtx_unlock(&proc->status_mtx);
}

struct monitor_buffer_node *attach_to_buffer(pid_t pid, pid_t tid, size_t size_in_pages)
{
	char name[SHM_NAME_MAXLEN];
	if (shm_mapname_thread_pid_tid(name, pid, tid) <= 0)
	{
		abort();
	}
	struct monitor_buffer_node *buffer_node = (struct monitor_buffer_node *)malloc(sizeof(struct monitor_buffer_node));
	monitor_buffer buffer = &buffer_node->buffer;
	buffer->fd = open(name, O_RDWR | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK, 0644);
	if (buffer->fd <= 0)
	{
		perror("could not open file");
		abort();
	}
	size_t alloc_size = size_in_pages * SHM_PAGE_SIZE;

	void *mem = mmap(0, alloc_size,
					 PROT_READ | PROT_WRITE, MAP_SHARED, buffer->fd, 0);

	buffer->buf_start = (buffer_entry *)mem;
	buffer->buf_pos = (buffer_entry *)mem;
	buffer->size_in_pages = size_in_pages;
	buffer->dbuf = NULL;
	buffer_node->tid=tid;

	size_t entry_count = alloc_size / sizeof(buffer_entry);
	buffer->buf_last = buffer->buf_start + (entry_count - 1);
	buffer->buf_prev = buffer->buf_last - 1;
	buffer->stopped = 0;
	return buffer_node;
}
databuffer *attach_to_databuffer(pid_t pid, pid_t tid, size_t size_in_pages, uint64_t bufid)
{
	char name[SHM_NAME_MAXLEN];
	if (shm_mapname_thread_data(name, pid, tid, bufid) <= 0)
	{
		abort();
	}
	databuffer *dbuf_node = (databuffer *)malloc(sizeof(databuffer));
	dbuf_node->id = bufid;
	dbuf_node->last_offset = 0;
	dbuf_node->size_in_pages = size_in_pages;

	int fd = open(name, O_RDWR | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK, 0644);
	if (fd <= 0)
	{
		perror("could not open file");
		abort();
	}
	unlink(name);
	size_t alloc_size = size_in_pages * SHM_PAGE_SIZE;

	void *mem = mmap(0, alloc_size,
					 PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	dbuf_node->fd = fd;
	dbuf_node->final_offset = alloc_size;
	dbuf_node->start = mem;

	return dbuf_node;
}

monitor_buffer find_thread_buffer(monitored_process proc, pid_t tid)
{
	struct monitor_buffer_node * current=proc->thread_buffers;
	if(current==NULL)
	{
		return NULL;
	}
	do
	{
		if(current->tid==tid)
		{
			return &current->buffer;
		}
		current=current->next;
	} while (current!=proc->thread_buffers);
	return NULL;
}

void insert_thread_buffer(monitored_process proc, struct monitor_buffer_node *node)
{
	if (proc->thread_buffers == NULL)
	{
		node->next = node;
		node->prev = node;
		proc->thread_buffers = node;
	}
	else
	{
		node->next = proc->thread_buffers;
		node->prev = proc->thread_buffers->prev;
		node->prev->next = node;
		node->next->prev = node;
	}
}

void insert_data_buffer(monitored_process proc, pid_t tid, databuffer * dbuf)
{
	monitor_buffer buf=find_thread_buffer(proc,tid);
	if(buf==NULL)
	{
        abort();
	}
	mtx_lock(&buf->data_mtx);
	if(buf->dbuf==NULL)
	{
		dbuf->next=dbuf;
		dbuf->prev=dbuf;
		buf->dbuf=dbuf;
	}
	else
	{
		databuffer* first_dbuf = buf->dbuf;
		dbuf->next=first_dbuf;
		dbuf->prev=first_dbuf->prev;
		dbuf->prev->next=dbuf;
		dbuf->next->prev=dbuf;
	}
	cnd_broadcast(&buf->data_cond);
	mtx_unlock(&buf->data_mtx);
}


void finalize_data_buffer(monitored_process proc, pid_t tid, uint64_t dbufid)
{

}

databuffer * progress_data_buffer(monitor_buffer buf, uint64_t dbufid)
{
	databuffer * first = buf->dbuf;
	if(first==NULL)
	{
		mtx_lock(&buf->data_mtx);
		first=buf->dbuf;
		while(first==NULL)
		{
			cnd_wait(&buf->data_cond, &buf->data_mtx);
			first=buf->dbuf;
		}
		mtx_unlock(&buf->data_mtx);
	}
	if(first->id==dbufid)
	{
		return first;
	}
	size_t page_size = SHM_PAGE_SIZE;
	mtx_lock(&buf->data_mtx);
	while(1)
	{
		first=buf->dbuf;
		databuffer * current=first;
		do
		{
			if(current->id==dbufid)
			{
				buf->dbuf=current;
				mtx_unlock(&buf->data_mtx);
				return current;
			}
			first->prev->next=current->next;
			current->next->prev=first->prev;
			munmap(current->start, current->size_in_pages * page_size);
			close(current->fd);
			databuffer *last=current;
			current=current->next;
			free(last);
		} while (current!=first);
		buf->dbuf=NULL;
		cnd_wait(&buf->data_cond, &buf->data_mtx);
	}
}

void remove_data_buffer(monitor_buffer buf, databuffer * dbuf)
{
	mtx_lock(&buf->data_mtx);
	if(buf->dbuf==dbuf)
	{
		if(dbuf->next==dbuf)
		{
			buf->dbuf=NULL;
		}
		else
		{
			buf->dbuf=dbuf->next;
		}
	}
	dbuf->next->prev=dbuf->prev;
	dbuf->prev->next=dbuf->next;
	munmap(dbuf->start, dbuf->size_in_pages*SHM_PAGE_SIZE);
	close(dbuf->fd);
	free(dbuf);
	mtx_unlock(&buf->data_mtx);
}

void stop_threads(monitored_process proc)
{
	struct monitor_buffer_node *current = proc->thread_buffers;
	if (current != NULL)
	{
		do
		{
			current->buffer.stopped = 1;
		} while (current->next != proc->thread_buffers);
	}
}

int main_process_event_loop(void *arg)
{
	buffer_entry entry;
	monitored_process proc = (monitored_process)arg;
	while (proc->status == 0)
	{
		if (copy_events_wait(&proc->buffer, &entry, 1) == 1)
		{
			switch (entry.kind)
			{
			case ABMGMT_CLOSE:
			{
				mtx_lock(&proc->status_mtx);
				proc->status = entry.payload32_1;
				stop_threads(proc);
				cnd_broadcast(&proc->status_cond);
				mtx_unlock(&proc->status_mtx);
			}
			break;
			case ABMGMT_NEW:
			{
				struct monitor_buffer_node *buffer_node = attach_to_buffer(proc->pid, entry.payload32_1, entry.payload64_1);
				insert_thread_buffer(proc, buffer_node);
				proc->register_thread_buffer(&buffer_node->buffer);
			}
			break;
			case ABMGMT_NEWDATA:
			{
				databuffer * dbuf = attach_to_databuffer(proc->pid, entry.payload32_1, entry.payload64_1, entry.payload64_2);
				insert_data_buffer(proc, entry.payload32_1, dbuf);
			}
			break;
			case ABMGMT_HELLO:
			{

			}
			break;
			}
		}
	}
}

monitored_process attach_to_process(pid_t pid, int (*register_thread_buffer)(monitor_buffer buffer))
{
	char name[SHM_NAME_MAXLEN];
	if (shm_mapname_thread_pid(name, pid) <= 0)
	{
		abort();
	}
	size_t mallocsize = sizeof(struct monitored_process);
	monitored_process buffer = (monitored_process)malloc(mallocsize);
	buffer->buffer.fd = open(name, O_RDWR | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK, 0644);
	while (buffer->buffer.fd <= 0)
	{
		buffer->buffer.fd = open(name, O_RDWR | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK, 0644);
	}
	size_t alloc_size = sysconf(_SC_PAGESIZE);

	void *mem = mmap(0, alloc_size,
					 PROT_READ | PROT_WRITE, MAP_SHARED, buffer->buffer.fd, 0);

	buffer->buffer.buf_start = (buffer_entry *)mem;
	buffer->buffer.buf_pos = (buffer_entry *)mem;
	buffer->buffer.size_in_pages = 1;

	size_t entry_count = alloc_size / sizeof(buffer_entry);
	buffer->buffer.buf_last = buffer->buffer.buf_start + (entry_count - 1);
	buffer->buffer.buf_prev = buffer->buffer.buf_last - 1;
	buffer->buffer.stopped = 0;
	buffer->register_thread_buffer = register_thread_buffer;
	buffer->thread_buffers=NULL;

	mtx_init(&buffer->status_mtx, mtx_plain);
	cnd_init(&buffer->status_cond);
	buffer->status = 0;
	buffer->pid = pid;
	thrd_create(&buffer->main_event_loop_thrd, &main_process_event_loop, (void *)buffer);

	return buffer;
}

size_t copy_events_wait(monitor_buffer buffer, buffer_entry *targetbuffers, size_t count)
{
	size_t actual = 0;
	buffer_entry *buffer_buffer=targetbuffers;
	buffer_entry *current_entry = buffer->buf_pos;
	buffer_entry_id entry_id;
	while (actual < count && !buffer->stopped)
	{
		entry_id = atomic_load_explicit(&current_entry->id, memory_order_acquire);
		if ((entry_id & (buffer_entry_id)1) == 0)
		{
			if (entry_id == BUF_ENTRY_MAGIC_ID)
			{
				current_entry = buffer->buf_start;
				continue;
			}
			else if (entry_id == BUF_ENTRY_FREE_ID)
			{
				if (actual > 0)
				{
					break;
				}
				spin_wait();
				entry_id = atomic_load_explicit(&current_entry->id, memory_order_acquire);
				while (entry_id == BUF_ENTRY_FREE_ID)
				{
					thrd_sleep(&(struct timespec){.tv_nsec = 1000}, NULL);
					if (buffer->stopped)
					{
						break;
					}
					entry_id = atomic_load_explicit(&current_entry->id, memory_order_acquire);
				}
				// if (buffer->stopped)
				// {
				// 	break;
				// }
			}
			else
			{
				abort();
			}
		}
		*buffer_buffer = *current_entry;
		//memcpy(buffer_buffer, current_entry, sizeof(buffer_entry));
		atomic_store_explicit(&current_entry->id, BUF_ENTRY_FREE_ID, memory_order_release);
		if (buffer_buffer->flags == TBMGMT_DATA)
		{
			databuffer * dbuf = progress_data_buffer(buffer, buffer_buffer->payload64_1);
			size_t dsize = buffer_buffer->payload64_2;
			void* newmem = malloc(dsize+1);
			memcpy(newmem, (void*)(((intptr_t)dbuf->start)+buffer_buffer->payload32_1), dsize);
			((char*)newmem)[dsize]=0;
			buffer_buffer->payload64_1=(intptr_t)newmem;
			dbuf->last_offset=buffer_buffer->payload64_1+dsize;
			buffer_buffer->payload32_1=0;
		}
		buffer_buffer++;
		current_entry++;
		actual++;
	}
	buffer->buf_pos = current_entry;
	return actual;
}

// Writes at least one, but up to count entries from the event buffer into buffer_buffer, except if the
// monitoring buffer has been stopped.
// With very high probability, all these events are in sequence (the event monitored application
// would have to go through all possible 2^63 event IDs at least once, possibly multiple times
// to align within the event buffer, in order for the events at the current read location to have
// the same IDs as in an earlier run again while this function is collecting events).
size_t copy_events_nowait(monitor_buffer buffer, buffer_entry *buffer_buffer, size_t count)
{
	size_t actual = 0;
	buffer_entry *current_entry = buffer->buf_pos;
	buffer_entry *prev_entry = buffer->buf_prev;
	buffer_entry_id entry_id;
	buffer_entry_id expected_entry_id = atomic_load_explicit(&prev_entry->id, memory_order_acquire) + 2;
	buffer_entry_id expected_entry_id_2 = expected_entry_id;
	while (actual < count && !buffer->stopped)
	{
		entry_id = atomic_load_explicit(&current_entry->id, memory_order_acquire);
		if (entry_id != expected_entry_id)
		{
			if (entry_id == BUF_ENTRY_MAGIC_ID)
			{
				current_entry = buffer->buf_start;
				continue;
			}
			else
			{
				expected_entry_id_2 = atomic_load_explicit(&prev_entry->id, memory_order_acquire) + 2;
				if (expected_entry_id_2 == expected_entry_id)
				{
					if (actual > 0)
					{
						break;
					}
					spin_wait();
					entry_id = atomic_load_explicit(&current_entry->id, memory_order_acquire);
					if (entry_id != expected_entry_id)
					{
						thrd_sleep(&(struct timespec){.tv_nsec = 1000}, NULL);
						continue;
					}
				}
				else
				{
					if (actual > 0)
					{
						break;
					}
					expected_entry_id = expected_entry_id_2;
					if (entry_id != expected_entry_id)
					{
						continue;
					}
				}
			}
		}
		// entry_id == expected_entry_id
		memcpy(buffer_buffer, current_entry, sizeof(buffer_entry));
		atomic_thread_fence(memory_order_release);
		expected_entry_id_2 = atomic_load_explicit(&prev_entry->id, memory_order_acquire) + 2;
		if (expected_entry_id_2 != expected_entry_id)
		{
			if (actual > 0)
			{
				break;
			}
			expected_entry_id = expected_entry_id_2;
			continue;
		}
		prev_entry = current_entry;
		if (buffer_buffer->flags == TBMGMT_DATA)
		{
			databuffer * dbuf = progress_data_buffer(buffer, buffer_buffer->payload64_1);
			size_t dsize = buffer_buffer->payload64_2;
			void* newmem = malloc(dsize+1);
			memcpy(newmem, (void*)(((intptr_t)dbuf->start)+buffer_buffer->payload32_1), dsize);
			((char*)newmem)[dsize]=0;
			buffer_buffer->payload64_1=(intptr_t)newmem;
			dbuf->last_offset=buffer_buffer->payload64_1+dsize;
			buffer_buffer->payload32_1=0;
		}
		buffer_buffer++;
		current_entry++;
		actual++;
	}
	buffer->buf_pos = current_entry;
	buffer->buf_prev = prev_entry;
	return actual;
}
