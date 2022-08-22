#include <time.h>
#include <unistd.h>

#include "utils.h"

int sleep_ns(uint64_t ns) {
    struct timespec ts = {.tv_nsec = ns};
    return nanosleep(&ts, NULL);
}

int sleep_ms(uint64_t ms) {
    return sleep_ns(ms * 1000000);
}
