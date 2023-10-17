#ifndef VAMOS_SIGNATURES_H_
#define VAMOS_SIGNATURES_H_

#include <stddef.h>
#include <stdint.h>

/*
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
    uint64_t t; /* timestamp */
    /* any of those types */
} signature_operand;

size_t signature_op_get_size(unsigned char c);
size_t signature_get_size(const unsigned char *sig);

#endif /* SIGNATURES_H_ */
