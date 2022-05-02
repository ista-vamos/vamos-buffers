#include <assert.h>
#include <stdlib.h>
#include "events.h"

size_t
call_event_op_get_size(char c) {
    switch(c) {
        case 'c': return sizeof(char);
        case 's': return sizeof(short);
        case 'i': return sizeof(int);
        case 'l': return sizeof(long);
        case 'f': return sizeof(float);
        case 'd': return sizeof(double);
        case 'p': return sizeof(void *);
        case 'S': return sizeof(uint64_t);
        case '_': return 0;
        default: assert(0 && "Invalid op type"); abort();
    };
}

size_t call_event_spec_get_size(struct call_event_spec *spec) {
    size_t n = 0;
    for (unsigned char *c = spec->signature; *c != 0; ++c) {
    	n += call_event_op_get_size(*c);
    }
    return n;
}

