#ifndef VAMOS_SOURCE_H_
#define VAMOS_SOURCE_H_

#include "event.h"

struct event_record {
    char name[64];
    vms_kind kind;
    size_t size;
    unsigned char signature[32];
};

struct vms_source_control {
    size_t size; /* size of this structure (it's variable) */
    struct event_record events[];
};

size_t vms_source_control_get_records_num(struct vms_source_control *sc);

struct vms_source_control *vms_source_control_define(size_t ev_nums, ...);
struct vms_source_control *vms_source_control_define_pairwise(
    size_t ev_nums, const char *names[], const char *signatures[]);

/* string is in the format "event-name:signature,event-name:signature,..." */
struct vms_source_control *vms_source_control_define_str(const char *str);

size_t vms_source_control_max_event_size(struct vms_source_control *control);

struct event_record *vms_source_control_get_event(
    struct vms_source_control *control, const char *name);

struct vms_source_control *vms_source_control_allocate(size_t ev_nums);

_Bool vms_source_control_define_pairwise_partially(
    struct vms_source_control *control, size_t from, size_t ev_nums,
    const char *names[], const char *signatures[]);

_Bool vms_source_control_define_partially(struct vms_source_control *control,
                                          size_t from, size_t ev_nums, ...);
#endif /* VAMOS_SOURCE_H_ */
