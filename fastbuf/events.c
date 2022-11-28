#include "events.h"
#include <assert.h>
#include <stdlib.h>

static inline unsigned short op_get_size(char c) {
    switch (c) {
    case 'c':
        return sizeof(char);
    case 's':
        return sizeof(short);
    case 'i':
        return sizeof(int);
    case 'l':
        return sizeof(long);
    case 'f':
        return sizeof(float);
    case 'd':
        return sizeof(double);
    case 'p':
        return sizeof(void *);
    case '_':
        return 0;
    default:
        assert(0 && "Invalid op type");
        abort();
    };
}

size_t call_event_spec_get_size(struct call_event_spec *spec) {
    size_t n = 0;
    for (unsigned char *c = spec->signature; *c != 0; ++c)
        n += op_get_size(*c);
    return 0;
}
