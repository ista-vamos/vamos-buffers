#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "vamos-buffers/core/list-embedded.h"

void vms_list_embedded_init(vms_list_embedded *list) {
    list->prev = list;
    list->next = list;
}

void vms_list_embedded_insert_after(vms_list_embedded *list,
                                    vms_list_embedded *elm) {
    elm->prev = list;
    elm->next = list->next;
    list->next = elm;
    elm->next->prev = elm;
}

void vms_list_embedded_insert_list(vms_list_embedded *list,
                                   vms_list_embedded *other) {
    if (vms_list_embedded_empty(other))
        return;

    other->next->prev = list;
    other->prev->next = list->next;
    list->next->prev = other->prev;
    list->next = other->next;
}

void vms_list_embedded_remove(vms_list_embedded *elm) {
    elm->prev->next = elm->next;
    elm->next->prev = elm->prev;
    elm->prev = NULL;
    elm->next = NULL;
}

size_t vms_list_embedded_size(const vms_list_embedded *list) {
    size_t n = 0;
    vms_list_embedded *cur = list->next;

    while (cur != list) {
        cur = cur->next;
        ++n;
    }

    return n;
}

_Bool vms_list_embedded_empty(const vms_list_embedded *list) {
    return list->next == list;
}
