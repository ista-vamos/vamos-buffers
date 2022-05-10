#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "client.h"
#include "utils.h"
#include "buffer.h"
#include "shm.h"

#define SLEEP_TIME 1000
void buffer_wait_for_monitor(struct buffer *buff) {
    while (!buffer_monitor_attached(buff)) {
        sleep_ms(SLEEP_TIME);
    }
}

