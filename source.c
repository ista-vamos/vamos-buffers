#include <assert.h>
#include <string.h>
#include <stdarg.h>

#include "source.h"
#include "signatures.h"

size_t source_control_get_records_num(struct source_control *sc) {
    return ((sc->size - sizeof(struct source_control))/sizeof(struct event_record));
}

struct source_control *source_control_define(size_t ev_nums, ...) {
    size_t control_size = sizeof(size_t) + ev_nums*sizeof(struct event_record);
    struct source_control *control = malloc(control_size);
    assert(control);

    control->size = control_size;

    va_list ap;
    va_start(ap, ev_nums);

    const size_t max_name_size = sizeof(control->events[0].name) - 1;
    const size_t max_sig_size = sizeof(control->events[0].signature) - 1;
    for (size_t i = 0; i < ev_nums; ++i) {
        const char *name = va_arg(ap, const char *);
        assert(strlen(name) <= max_name_size);
        strncpy(control->events[i].name, name, max_name_size);
        control->events[i].name[max_name_size] = '\0';

        const char *sig = va_arg(ap, const char *);
        assert(strlen(sig) <= max_sig_size);
        strncpy((char*)control->events[i].signature, sig, max_sig_size);
        control->events[i].signature[max_sig_size] = '\0';

        control->events[i].size
                = signature_get_size((const unsigned char *)sig) + sizeof(shm_event);
        control->events[i].kind = 0;

    }

    va_end(ap);

    return control;
}
