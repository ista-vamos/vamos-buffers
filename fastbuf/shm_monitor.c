#define _GNU_SOURCE
#include "shm_monitor.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <threads.h>
#ifdef LINUX
#include <sys/types.h>
#include <sys/file.h>
#include <sys/mman.h>
//TODO: handle windows case
#endif

struct monitor_buffer
{
	buffer_entry *buf_start;
	buffer_entry *buf_pos;
	buffer_entry *buf_prev;
	buffer_entry *buf_last;
	int fd;
	size_t size_in_pages;
	int stopped;
};

static int shamon_shm_unlink(monitor_buffer buffer, pid_t pid, pid_t tid)
{
	char name[SHM_NAME_MAXLEN];
	if (shm_mapname_thread_pid_tid(name, pid, tid) == 0)
	{
		abort();
	}
	return unlink(name);
}

int attach_to_buffer(monitor_buffer buffer, pid_t pid, pid_t tid, size_t size_in_pages)
{
	char name[SHM_NAME_MAXLEN];
	if (shm_mapname_thread_pid_tid(name, pid, tid) == 0)
	{
		abort();
	}
	buffer->fd = open(name, O_RDWR | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK, 0);
	if (!buffer->fd)
	{
		abort();
	}
	size_t alloc_size = size_in_pages * sysconf(_SC_PAGESIZE);

	void *mem = mmap(0, alloc_size,
					 PROT_READ | PROT_WRITE, MAP_SHARED, buffer->fd, 0);

	buffer->buf_start = (buffer_entry *)mem;
	buffer->buf_pos = (buffer_entry *)mem;
	buffer->size_in_pages = size_in_pages;

	size_t entry_count = alloc_size / sizeof(buffer_entry);
	buffer->buf_last = buffer->buf_start + (entry_count - 1);
	buffer->buf_prev = buffer->buf_last - 1;
	buffer->stopped=0;
}

size_t copy_events_wait(monitor_buffer buffer, buffer_entry *buffer_buffer, size_t count)
{
	size_t actual = 0;
	buffer_entry *current_entry = buffer->buf_pos;
	buffer_entry_id entry_id;
	while (actual < count&&!buffer->stopped)
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
					if(buffer->stopped)
					{
						break;
					}
					entry_id = atomic_load_explicit(&current_entry->id, memory_order_acquire);
				}
				if(buffer->stopped)
				{
					break;
				}
			}
			else
			{
				abort();
			}
		}
		memcpy(buffer_buffer, current_entry, sizeof(buffer_entry));
		atomic_store_explicit(&current_entry->id, BUF_ENTRY_FREE_ID, memory_order_release);
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
	while (actual < count&&!buffer->stopped)
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
				if(expected_entry_id_2==expected_entry_id)
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
					if(actual>0)
					{
						break;
					}
					expected_entry_id=expected_entry_id_2;
					if(entry_id!=expected_entry_id)
					{
						continue;
					}
				}
			}
		}
		// entry_id == expected_entry_id
		memcpy(buffer_buffer, current_entry, sizeof(buffer_entry));
		atomic_thread_fence(memory_order_release);
		expected_entry_id_2=atomic_load_explicit(&prev_entry->id, memory_order_acquire)+2;
		if(expected_entry_id_2!=expected_entry_id)
		{
			if(actual>0)
			{
				break;
			}
			expected_entry_id=expected_entry_id_2;
			continue;
		}
		prev_entry = current_entry;
		buffer_buffer++;
		current_entry++;
		actual++;
	}
	buffer->buf_pos = current_entry;
	buffer->buf_prev = prev_entry;
	return actual;
}