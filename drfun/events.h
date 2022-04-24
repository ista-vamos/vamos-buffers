#ifndef DRFUN_EVENTS_H_
#define DRFUN_EVENTS_H_

#include <stddef.h>

union shamon_operand {
    unsigned char c;
    unsigned short s;
    unsigned int i;
    unsigned long l;
    float f;
    double d;
    void *p;
};


struct call_event_spec {
    const char *file; /* module (UNUSED atm) */

    /* we need one of these */
    char *name; /* name */
    size_t addr;      /* offset in module */

    /* string describing what arguments to track and what is their size:
     * c = (unsigned) char
     * s = (unsigned) short
     * i = (unsigned) int
     * l = (unsigned) long
     * f = float
     * d = double
     * p = pointer
     * _ = skip argument
     * E.g.: "i_c" means track first and third arguments that have 4 bytes
     * and 1 byte size
     * TODO: add S for string (which will be copied into an another piece
     * of shared memory)
     * */
    unsigned char signature[16]; /* for now, we allow 16 arguments at most */
};

size_t call_event_op_get_size(char c);
size_t call_event_spec_get_size(struct call_event_spec *spec);

/*  just an auxiliar struct for now */
struct call_event_1i {
    size_t addr;
    unsigned int argument;
};


#endif /* DRFUN_EVENTS_H_ */
