#include "vamos-buffers/core/shm_string.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "vamos-buffers/core/shm_string-macro.h"

void shm_string_init(shm_string *s) {
    STRING_INIT(s->data);
    memset(s, 0, sizeof(*s));
}

void shm_string_destroy(shm_string *s) { STRING_DESTROY(s->data); }

void shm_string_swap(shm_string *s, shm_string *with) {
    char *data = s->data;
    size_t size = STRING_SIZE(s->data);
    size_t asize = STRING_ALLOC_SIZE(s->data);

    s->data = with->data;
    STRING_SIZE(s->data) = STRING_SIZE(with->data);
    STRING_ALLOC_SIZE(s->data) = STRING_ALLOC_SIZE(with->data);
    with->data = data;
    STRING_SIZE(with->data) = size;
    STRING_ALLOC_SIZE(with->data) = asize;
}

void shm_string_grow(shm_string *s, size_t size) {
    if (STRING_SIZE(s->data) >= size)
        return;

    STRING_GROW(s->data, size);
}

char *shm_string_extend(shm_string *s) {
    char *ret;
    STRING_EXTEND(s->data, ret);
    return ret;
}

size_t shm_string_append(shm_string *s, char c) {
    STRING_APPEND(s->data, c);
    return STRING_SIZE(s->data);
}

size_t shm_string_pop(shm_string *s) { return --STRING_SIZE(s->data); }

void shm_string_clear(shm_string *s) { STRING_SIZE(s->data) = 0; }

size_t shm_string_size(shm_string *s) { return STRING_SIZE(s->data); }

/*
 * Return the pointer to the element at index 'idx'
 */
char *shm_string_at_ptr(shm_string *s, ssize_t idx) {
    if (idx < 0) {
        assert((size_t)(-idx) <= STRING_SIZE(s->data) && "idx OOB");
        return &s->data[STRING_SIZE(s->data) + idx];
    }
    assert((size_t)idx < STRING_SIZE(s->data) && "idx OOB");
    return &s->data[idx];
}

char shm_string_at(shm_string *s, ssize_t idx) {
    return *shm_string_at_ptr(s, idx);
}
