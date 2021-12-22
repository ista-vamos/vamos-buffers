#define _GNU_SOURCE
#include "shm_common.h"
#include <stddef.h>
#include <threads.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef LINUX
#include <sys/types.h>
#include <sys/file.h>
#include <sys/mman.h>
//TODO: handle windows case
#endif

// SHARED

char *shm_mapname_thread_pid_tid(char *buf, pid_t pid, pid_t tid)
{
	if (snprintf(buf, SHM_NAME_MAXLEN, "/dev/shm/%i.%i", pid, tid))
	{
		return NULL;
	}
	return buf;
}

