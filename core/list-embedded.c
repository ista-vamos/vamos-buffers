#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "list-embedded.h"

void shm_list_embedded_init(shm_list_embedded *list) {
    list->prev = list;
    list->next = list;
}

void shm_list_embedded_insert_after(shm_list_embedded *list,
                                    shm_list_embedded *elm) {
    elm->prev       = list;
    elm->next       = list->next;
    list->next      = elm;
    elm->next->prev = elm;
}

void shm_list_embedded_insert_list(shm_list_embedded *list,
                                   shm_list_embedded *other) {
    if (shm_list_embedded_empty(other))
        return;

    other->next->prev = list;
    other->prev->next = list->next;
    list->next->prev  = other->prev;
    list->next        = other->next;
}

void shm_list_embedded_remove(shm_list_embedded *elm) {
    elm->prev->next = elm->next;
    elm->next->prev = elm->prev;
    elm->prev       = NULL;
    elm->next       = NULL;
}

size_t shm_list_embedded_size(const shm_list_embedded *list) {
    size_t             n   = 0;
    shm_list_embedded *cur = list->next;

    while (cur != list) {
        cur = cur->next;
        ++n;
    }

    return n;
}

_Bool shm_list_embedded_empty(const shm_list_embedded *list) {
    return list->next == list;
}
