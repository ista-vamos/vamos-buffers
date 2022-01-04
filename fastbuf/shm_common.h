#pragma once
#include <inttypes.h>
#include <stdatomic.h>
#include <unistd.h>

typedef uint64_t buffer_entry_id;
typedef uint16_t buffer_entry_kind;
typedef uint16_t buffer_kind;

#define BUF_ENTRY_MAGIC_ID 2
#define BUF_ENTRY_FREE_ID 0

#define SHM_NAME_MAXLEN 128
#define LINUX

typedef struct buffer_entry
{
	_Atomic buffer_entry_id id;
	buffer_entry_kind kind;
	int16_t payload16_1;
	int32_t payload32_1;
	int64_t payload64_1;
	int64_t payload64_2;
} buffer_entry;

char *shm_mapname_thread_pid(char *buf, pid_t pid);
char *shm_mapname_thread_pid_tid(char *buf, pid_t pid, pid_t tid);


static void spin_wait()
{
	for (int i = 0; i < 500; i++)
		;
}