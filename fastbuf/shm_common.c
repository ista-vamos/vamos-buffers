#define _GNU_SOURCE
#include "shm_common.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <unistd.h>
#ifdef LINUX
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/types.h>
// TODO: handle windows case
#endif

// SHARED

char *shm_mapname_thread_pid(char *buf, pid_t pid) {
    if (snprintf(buf, SHM_NAME_MAXLEN, "/dev/shm/%i.mon", pid) <= 0) {
        return NULL;
    }
    return buf;
}

char *shm_mapname_thread_pid_tid(char *buf, pid_t pid, pid_t tid) {
    if (snprintf(buf, SHM_NAME_MAXLEN, "/dev/shm/%i.%i.mon", pid, tid) <= 0) {
        return NULL;
    }
    return buf;
}

char *shm_mapname_thread_data(char *buf, pid_t pid, pid_t tid,
                              uint64_t dbufid) {
    if (snprintf(buf, SHM_NAME_MAXLEN, "/dev/shm/%i.%i.%lu.mon", pid, tid,
                 dbufid) <= 0) {
        return NULL;
    }
    return buf;
}

void spin_wait() {
    for (int i = 0; i < 500; i++)
        ;
}
