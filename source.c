#include <assert.h>
#include <string.h>
#include <stdarg.h>

#include "source.h"
#include "signatures.h"

size_t source_control_get_records_num(struct source_control *sc) {
    return ((sc->size - sizeof(struct source_control))/sizeof(struct event_record));
}

size_t source_control_max_event_size(struct source_control *control) {
    const size_t en = source_control_get_records_num(control);
    size_t max_size = 0;
    for (size_t i = 0; i < en; ++i) {
        if (max_size < control->events[i].size)
            max_size = control->events[i].size;
    }
    return max_size;
}

static inline void init_record(struct event_record *ev, const char *name, const char *sig) {
    const size_t max_name_size = sizeof(ev->name) - 1;
    const size_t max_sig_size = sizeof(ev->signature) - 1;

    assert(strlen(name) <= max_name_size);
    strncpy(ev->name, name, max_name_size);
    ev->name[max_name_size] = '\0';

    assert(strlen(sig) <= max_sig_size);
    strncpy((char*)ev->signature, sig, max_sig_size);
    ev->signature[max_sig_size] = '\0';

    ev->size = signature_get_size((const unsigned char *)sig) + sizeof(shm_event);
    ev->kind = 0;
}

struct source_control *source_control_define(size_t ev_nums, ...) {
    size_t control_size = sizeof(size_t) + ev_nums*sizeof(struct event_record);
    struct source_control *control = malloc(control_size);
    assert(control);

    control->size = control_size;

    va_list ap;
    va_start(ap, ev_nums);

    for (size_t i = 0; i < ev_nums; ++i) {
        const char *name = va_arg(ap, const char *);
        const char *sig = va_arg(ap, const char *);
        init_record(control->events + i, name, sig);
    }

    va_end(ap);

    return control;
}

struct source_control *source_control_define_pairwise(size_t ev_nums,
                                                      const char *names[],
                                                      const char *signatures[]) {
    size_t control_size = sizeof(size_t) + ev_nums*sizeof(struct event_record);
    struct source_control *control = malloc(control_size);
    assert(control);

    control->size = control_size;

    for (size_t i = 0; i < ev_nums; ++i) {
        init_record(control->events + i, names[i], signatures[i]);

    }

    return control;
}
