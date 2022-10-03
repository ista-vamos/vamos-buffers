#include <time.h>
#include <assert.h>
#include <unistd.h>

#include "utils.h"

int sleep_ns(uint64_t ns) {
    struct timespec ts = {0};
    if (ns > 999999999) {
        ts.tv_sec = ns / 1000000000;
        ns %= 1000000000;
    }

    assert (ns <= 999999999);
    ts.tv_nsec = ns;

    return nanosleep(&ts, NULL);
}

int sleep_ms(uint64_t ms) {
    return sleep_ns(ms * 1000000);
}
