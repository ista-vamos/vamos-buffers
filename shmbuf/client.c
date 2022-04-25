#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "client.h"
#include "buffer.h"
#include "shm.h"

#define SLEEP_TIME 1000
void buffer_wait_for_monitor(struct buffer *buff) {
    while (!buffer_monitor_attached(buff)) {
        if (usleep(SLEEP_TIME) == -1) {
        	perror("usleep error");
        }
    }
}

