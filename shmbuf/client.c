#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "buffer.h"
#include "client.h"
#include "shm.h"
#include "utils.h"

#define SLEEP_TIME 1000
int buffer_wait_for_monitor(struct buffer *buff) {
    while (!buffer_monitor_attached(buff)) {
        if (sleep_ms(SLEEP_TIME) != 0) {
            return -errno;
        }
    }
    /* sleep once more so that the monitor has some time
     * to move to monitoring code */
    if (sleep_ms(SLEEP_TIME) != 0)
        return -errno;
    return 0;
}
