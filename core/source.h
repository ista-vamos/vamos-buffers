#ifndef SHAMON_SOURCE_H_
#define SHAMON_SOURCE_H_

#include "event.h"

struct event_record {
    char          name[64];
    shm_kind      kind;
    size_t        size;
    unsigned char signature[32];
};

struct source_control {
    size_t              size; /* size of this structure (it's variable) */
    struct event_record events[];
};

size_t source_control_get_records_num(struct source_control *sc);

struct source_control *source_control_define(size_t ev_nums, ...);
struct source_control *source_control_define_pairwise(size_t      ev_nums,
                                                      const char *names[],
                                                      const char *signatures[]);

/* string is in the format "event-name:signature,event-name:signature,..." */
struct source_control *source_control_define_str(const char *str);

size_t source_control_max_event_size(struct source_control *control);

struct event_record *source_control_get_event(struct source_control *control,
                                              const char            *name);

struct source_control *source_control_allocate(size_t ev_nums);

_Bool source_control_define_pairwise_partially(struct source_control *control,
                                               size_t from, size_t ev_nums,
                                               const char *names[],
                                               const char *signatures[]);

_Bool source_control_define_partially(struct source_control *control,
                                      size_t from, size_t ev_nums, ...);
#endif /* SHAMON_SOURCE_H_ */
