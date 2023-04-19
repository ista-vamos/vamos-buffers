#include "vamos-buffers/shmbuf/client.h"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "shm.h"
#include "vamos-buffers/core/utils.h"
#include "vamos-buffers/shmbuf/buffer.h"

#define SLEEP_TIME 20

static volatile sig_atomic_t interrupted = 0;

static void sig_int(int signo) {
    (void)signo;
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
    int err = 0;

    while (!buffer_monitor_attached(buff)) {
        if (sleep_ms(SLEEP_TIME) != 0) {
            err = -errno;
            break;
        }
        if (interrupted) {
            err = -EINTR;
            break;
        }
    }

    restore_sigfunc(sigfunc);

    if (err < 0)
        return err;

    /* sleep once more so that the monitor has some time
     * to move to monitoring code */
    if (sleep_ms(SLEEP_TIME) != 0) {
        err = -errno;
    }

    return err;
}
