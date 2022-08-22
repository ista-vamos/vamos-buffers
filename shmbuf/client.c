#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "buffer.h"
#include "client.h"
#include "shm.h"
#include "utils.h"

#define SLEEP_TIME 1000

static volatile sig_atomic_t interrupted = 0;

static void sig_int(int signo) {
    (void) signo;
    interrupted = 1;
}

static inline void restore_sigfunc(void (*sigfunc)(int)) {
    if (sigfunc != SIG_ERR) {
        signal(SIGINT, sigfunc);
    }
}

int buffer_wait_for_monitor(struct buffer *buff) {
    void (*sigfunc)(int);
    sigfunc = signal(SIGINT, sig_int);

    while (!buffer_monitor_attached(buff)) {
        if (sleep_ms(SLEEP_TIME) != 0) {
            restore_sigfunc(sigfunc);
            return -errno;
        }
        if (interrupted) {
            restore_sigfunc(sigfunc);
            return -EINTR;
        }
    }

    restore_sigfunc(sigfunc);

    /* sleep once more so that the monitor has some time
     * to move to monitoring code */
    if (sleep_ms(SLEEP_TIME) != 0) {
        return -errno;
    }

    return 0;
}
