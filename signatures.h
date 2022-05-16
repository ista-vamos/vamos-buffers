#ifndef SHAMON_SIGNATURES_H_
#define SHAMON_SIGNATURES_H_

#include <stddef.h>
#include <stdint.h>

typedef union _signature_operand {
    unsigned char c;
    unsigned short h;
    unsigned int i;
    unsigned long l;
    float f;
    double d;
    void *p;
    union {
        /* shared string is a 32bit id of shared memory
         * block and 32bit offset */
        uint64_t shared;
        const char *local;
    } S;
    /* any of those types */
} signature_operand;

size_t signature_op_get_size(unsigned char c);
size_t signature_get_size(const unsigned char *sig);

#endif /* SIGNATURES_H_ */
