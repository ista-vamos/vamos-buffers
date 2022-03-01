#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include "client.h"
#include "shm.h"

/* TODO: make these operations atomic once we have multiple monitors/clients */

#define SLEEP_TIME 1000
void buffer_wait_for_monitor(struct buffer *buff) {
	while (buffer_get_monitors_num(buff) == 0) {
		if (usleep(SLEEP_TIME) == -1) {
			perror("usleep error");
		}
	}
}

