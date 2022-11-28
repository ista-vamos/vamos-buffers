#pragma once
#include <inttypes.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <unistd.h>

typedef uint64_t buffer_entry_id;
typedef uint16_t buffer_entry_kind;
typedef uint16_t buffer_kind;

#define BUF_ENTRY_MAGIC_ID 2
#define BUF_ENTRY_FREE_ID  0

#define SHM_NAME_MAXLEN 128

#define SHM_PAGE_SIZE (sysconf(_SC_PAGESIZE))

enum appbuffer_management_msgkind {
    ABMGMT_NONE,
    ABMGMT_HELLO,
    ABMGMT_CLOSE,
    ABMGMT_NEW,
    ABMGMT_NEWDATA,
    ABMGMT_CLOSEDATA
};
enum threadbuffer_management_msgflags { TBMGMT_STD, TBMGMT_DATA };

typedef struct buffer_entry {
    _Atomic buffer_entry_id id;
    buffer_entry_kind       flags;
    buffer_entry_kind       kind;
    int32_t                 payload32_1;
    int64_t                 payload64_1;
    int64_t                 payload64_2;
} buffer_entry;

char *shm_mapname_thread_pid(char *buf, pid_t pid);
char *shm_mapname_thread_pid_tid(char *buf, pid_t pid, pid_t tid);
char *shm_mapname_thread_data(char *buf, pid_t pid, pid_t tid, uint64_t dbufid);

void spin_wait();
