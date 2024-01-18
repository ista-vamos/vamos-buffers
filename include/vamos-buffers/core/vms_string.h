#ifndef VAMOS_STRING_H_
#define VAMOS_STRING_H_

#include <unistd.h>

#include "vms_string-macro.h"

typedef struct _vms_string_ref {
    size_t size;
    const char *data;
} vms_string_ref;

typedef struct _vms_string {
    STRING(data);
} vms_string;

/* Either the user can manipulate the string manually,
 * with the macros from vms_string-macro.h
 * or with the functions below. The macros are used
   on the data of the string, e.g.,
   STRING_RESIZE(s->data). Note that macros are not applicable
   always (they just expand to code that may be syntactically wrong
   in some contexts). */
void vms_string_init(vms_string *s);
void vms_string_destroy(vms_string *s);
void vms_string_clear(vms_string *s);
size_t vms_string_append(vms_string *s, char c);
char *vms_string_extend(vms_string *s);
size_t vms_string_pop(vms_string *s);
size_t vms_string_size(vms_string *s);
char *vms_string_at_ptr(vms_string *s, ssize_t idx);
char vms_string_at(vms_string *s, ssize_t idx);
void vms_string_swap(vms_string *s, vms_string *with);
void vms_string_grow(vms_string *s, size_t size);

#endif /* VAMOS_VECTOR_H_ */
