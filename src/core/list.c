#include "vamos-buffers/core/list.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void vms_list_init(vms_list *list) { memset(list, 0, sizeof(vms_list)); }

void vms_list_destroy(vms_list *list, vms_list_elem_destroy_fn destroy) {
    vms_list_elem *cur = list->first;
    while (cur) {
        if (destroy)
            destroy(cur->data);
        cur = cur->next;
        free(cur);
    }
}

size_t vms_list_insert_elem_after(vms_list *list, vms_list_elem *elem,
                                  vms_list_elem *new) {
    assert(elem);
    new->next = elem->next;
    new->prev = elem;
    if (elem->next)
        elem->next->prev = new;
    elem->next = new;
    if (elem == list->last) {
        list->last = new;
    }

    assert(!list->first == !list->last);
    return ++list->size;
}

size_t vms_list_insert_after(vms_list *list, vms_list_elem *elem, void *data) {
    assert(elem);
    vms_list_elem *new = malloc(sizeof(*new));
    assert(new);
    new->data = data;
    return vms_list_insert_elem_after(list, elem, new);
}

size_t vms_list_insert_before(vms_list *list, vms_list_elem *elem, void *data) {
    assert(elem);
    vms_list_elem *new = malloc(sizeof(*new));
    assert(new);
    new->data = data;
    new->next = elem;
    if (elem->prev)
        elem->prev->next = new;
    new->prev = elem->prev;
    elem->prev = new;

    if (elem == list->first) {
        assert(new->prev == NULL);
        list->first = new;
    }

    assert(!list->first == !list->last);
    return ++list->size;
}

size_t vms_list_prepend(vms_list *list, void *data) {
    assert(!list->first || list->first->prev == NULL);
    vms_list_elem *new = malloc(sizeof(*new));
    assert(new);
    new->data = data;
    new->next = list->first;
    new->prev = NULL;
    list->first = new;
    if (!list->last)
        list->last = new;

    assert(!list->first == !list->last);
    return ++list->size;
}

size_t vms_list_append(vms_list *list, void *data) {
    assert(list->last == NULL || list->last->next == NULL);
    vms_list_elem *new = malloc(sizeof(*new));
    assert(new);
    new->data = data;
    new->next = NULL;
    new->prev = list->last;
    if (list->last)
        list->last->next = new;
    list->last = new;
    if (!list->first)
        list->first = new;

    assert(!list->first == !list->last);
    return ++list->size;
}

size_t vms_list_append_elem(vms_list *list, vms_list_elem *elem) {
    assert(list->last == NULL || list->last->next == NULL);
    assert(elem);
    elem->next = NULL;
    elem->prev = list->last;
    if (list->last)
        list->last->next = elem;
    list->last = elem;
    if (!list->first)
        list->first = elem;

    assert(!list->first == !list->last);
    return ++list->size;
}

size_t vms_list_remove(vms_list *list, vms_list_elem *elem) {
    assert(elem);
    assert(list->size > 0);
    assert(list->first);
    assert(list->last);

    /*
    if (list->size == 1) {
        assert(elem == list->first);
        assert(elem == list->last);
        list->first = list->last = NULL;
        list->size = 0;
        return 0;
    }
    */

    /* got at least two elements
    assert(list->last != list->first);
    */
    if (elem->next) {
        elem->next->prev = elem->prev;
    }
    if (elem->prev) {
        elem->prev->next = elem->next;
    }
    if (elem == list->first) {
        list->first = elem->next;
    }
    if (elem == list->last) {
        list->last = elem->prev;
    }

    assert(!list->first == !list->last);
    return --list->size;
}

size_t vms_list_size(vms_list *list) {
    assert(list->size > 0 || (!list->first && !list->last));
    assert(!list->first == !list->last);
    return list->size;
}

vms_list_elem *vms_list_first(vms_list *list) {
    assert(!list->first == !list->last);
    return list->first;
}

vms_list_elem *vms_list_last(vms_list *list) {
    assert(!list->first == !list->last);
    return list->last;
}
