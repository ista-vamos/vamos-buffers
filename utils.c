#include <time.h>
#include <unistd.h>
#include "utils.h"

void sleep_ns(uint64_t ns) {
    struct timespec ts = { .tv_nsec = ns };
    nanosleep(&ts, NULL);
}

void sleep_ms(uint64_t ms) {
    sleep_ns(ms * 1000000);
}

