#include <stdlib.h>
#include <stdio.h>
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


void *xalloc_aligned(size_t size, size_t alignment) {
    assert(size > 0);
    assert(alignment > 0);

    void *mem;
    int succ = posix_memalign(&mem, alignment, size);
    if (succ != 0) {
        perror("posix_memalign");
        assert(0 && "Allocation failed");
        abort();
    }

    return mem;
}
