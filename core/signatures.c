#include "signatures.h"

#include <assert.h>
#include <stdlib.h>

size_t signature_op_get_size(unsigned char c) {
    signature_operand op;
    switch (c) {
        case 'c':
            return sizeof(op.c);
        case 'h':
            return sizeof(op.h);
        case 'i':
            return sizeof(op.i);
        case 'l':
            return sizeof(op.l);
        case 'f':
            return sizeof(op.l);
        case 'd':
            return sizeof(op.l);
        case 'p':
            return sizeof(op.p);
        case 't':
            return sizeof(op.t);
        case 'S':
        case 'L': /* aliases for strings: line and match */
        case 'M':
            return sizeof(op.S);
        /* unknown, any of possible types */
        case '?':
            return sizeof(op);
        case '_':
            return 0; /* skip operand */
        default:
            assert(0 && "Invalid op type");
            abort();
    };
}

size_t signature_get_size(const unsigned char *sig) {
    size_t n = 0;
    for (const unsigned char *c = sig; *c != 0; ++c) {
        n += signature_op_get_size(*c);
    }
    return n;
}
