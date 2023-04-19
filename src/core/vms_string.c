#include "vamos-buffers/core/vms_string.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "vamos-buffers/core/vms_string-macro.h"

void vms_string_init(vms_string *s) {
    STRING_INIT(s->data);
    memset(s, 0, sizeof(*s));
}

void vms_string_destroy(vms_string *s) { STRING_DESTROY(s->data); }

void vms_string_swap(vms_string *s, vms_string *with) {
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

void vms_string_grow(vms_string *s, size_t size) {
    if (STRING_SIZE(s->data) >= size)
        return;

    STRING_GROW(s->data, size);
}

char *vms_string_extend(vms_string *s) {
    char *ret;
    STRING_EXTEND(s->data, ret);
    return ret;
}

size_t vms_string_append(vms_string *s, char c) {
    STRING_APPEND(s->data, c);
    return STRING_SIZE(s->data);
}

size_t vms_string_pop(vms_string *s) { return --STRING_SIZE(s->data); }

void vms_string_clear(vms_string *s) { STRING_SIZE(s->data) = 0; }

size_t vms_string_size(vms_string *s) { return STRING_SIZE(s->data); }

/*
 * Return the pointer to the element at index 'idx'
 */
char *vms_string_at_ptr(vms_string *s, ssize_t idx) {
    if (idx < 0) {
        assert((size_t)(-idx) <= STRING_SIZE(s->data) && "idx OOB");
        return &s->data[STRING_SIZE(s->data) + idx];
    }
    assert((size_t)idx < STRING_SIZE(s->data) && "idx OOB");
    return &s->data[idx];
}

char vms_string_at(vms_string *s, ssize_t idx) {
    return *vms_string_at_ptr(s, idx);
}
