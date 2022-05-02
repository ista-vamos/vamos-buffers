#ifndef DRFUN_EVENTS_H_
#define DRFUN_EVENTS_H_

#include <stddef.h>
#include <stdint.h>

union shamon_operand {
    unsigned char c;
    unsigned short s;
    unsigned int i;
    unsigned long l;
    float f;
    double d;
    void *p;
    /* string is a 32bit id of shared memory block and 32bit offset */
    uint64_t S;
};


struct call_event_spec {
    char name[256]; /* name */
    size_t addr;    /* offset in module */

    /* string describing what arguments to track and what is their size:
     * c = (unsigned) char
     * s = (unsigned) short
     * i = (unsigned) int
     * l = (unsigned) long
     * f = float
     * d = double
     * p = pointer
     * S = string (0-terminated array of chars)
     * _ = skip argument
     * E.g.: "i_c" means track first and third arguments that have 4 bytes
     * and 1 byte size
     * */
    unsigned char signature[16]; /* for now, we allow 16 arguments at most */
    /* the type of event assigned by the monitor */
    uint64_t kind;
};

size_t call_event_op_get_size(char c);
size_t call_event_spec_get_size(struct call_event_spec *spec);


#endif /* DRFUN_EVENTS_H_ */
