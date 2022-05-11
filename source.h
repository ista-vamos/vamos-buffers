#ifndef SHAMON_SOURCE_H_
#define SHAMON_SOURCE_H_

#include "event.h"

struct event_record {
    char name[64];
    shm_kind kind;
    size_t size;
    unsigned char signature[32];
};

struct source_control {
    size_t size; /* size of this structure (its variable) */
    struct event_record events[];
};

#endif  /* SHAMON_SOURCE_H_ */
