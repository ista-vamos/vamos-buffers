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
    size_t size; /* size of this structure (it's variable) */
    struct event_record events[];
};


size_t source_control_get_records_num(struct source_control *sc);

struct source_control *source_control_define(size_t ev_nums, ...);
struct source_control *source_control_define_pairwise(size_t ev_nums,
                                                      const char *names[],
                                                      const char *signatures[]);


#endif  /* SHAMON_SOURCE_H_ */
