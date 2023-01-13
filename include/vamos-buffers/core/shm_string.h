#ifndef SHAMON_STRING_H_
#define SHAMON_STRING_H_

#include <unistd.h>

#include "shm_string-macro.h"

typedef struct _shm_string_ref {
    size_t size;
    const char *data;
} shm_string_ref;

typedef struct _shm_string {
    STRING(data);
} shm_string;

/* Either the user can manipulate the string manually,
 * with the macros from shm_string-macro.h
 * or with the functions below. The macros are used
   on the data of the string, e.g.,
   STRING_RESIZE(s->data). Note that macros are not applicable
   always (they just expand to code that may be syntactically wrong
   in some contexts). */
void shm_string_init(shm_string *s);
void shm_string_destroy(shm_string *s);
void shm_string_clear(shm_string *s);
size_t shm_string_append(shm_string *s, char c);
char *shm_string_extend(shm_string *s);
size_t shm_string_pop(shm_string *s);
size_t shm_string_size(shm_string *s);
char *shm_string_at_ptr(shm_string *s, ssize_t idx);
char shm_string_at(shm_string *s, ssize_t idx);
void shm_string_swap(shm_string *s, shm_string *with);
void shm_string_grow(shm_string *s, size_t size);

#endif /* SHAMON_VECTOR_H_ */
