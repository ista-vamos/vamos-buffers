#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

void shm_list_init(shm_list *list) {
    memset(list, 0, sizeof(shm_list));
}

void shm_list_destroy(shm_list *list, shm_list_elem_destroy_fn destroy) {
    shm_list_elem *cur = list->first;
    while (cur) {
        if (destroy)
            destroy(cur->data);
        cur = cur->next;
        free(cur);
    }
}

size_t shm_list_insert_elem_after(shm_list *list, shm_list_elem *elem,
                                  shm_list_elem *new) {
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

size_t shm_list_insert_after(shm_list *list, shm_list_elem *elem, void *data) {
    assert(elem);
    shm_list_elem *new = malloc(sizeof(*new));
    assert(new);
    new->data = data;
    return shm_list_insert_elem_after(list, elem, new);
}

size_t shm_list_insert_before(shm_list *list, shm_list_elem *elem, void *data) {
    assert(elem);
    shm_list_elem *new = malloc(sizeof(*new));
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

size_t shm_list_prepend(shm_list *list, void *data) {
    assert(!list->first || list->first->prev == NULL);
    shm_list_elem *new = malloc(sizeof(*new));
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

size_t shm_list_append(shm_list *list, void *data) {
    assert(list->last == NULL || list->last->next == NULL);
    shm_list_elem *new = malloc(sizeof(*new));
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

size_t shm_list_append_elem(shm_list *list, shm_list_elem *elem) {
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

size_t shm_list_remove(shm_list *list, shm_list_elem *elem) {
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

size_t shm_list_size(shm_list *list) {
    assert(list->size > 0 || (!list->first && !list->last));
    assert(!list->first == !list->last);
    return list->size;
}

shm_list_elem *shm_list_first(shm_list *list) {
    assert(!list->first == !list->last);
    return list->first;
}

shm_list_elem *shm_list_last(shm_list *list) {
    assert(!list->first == !list->last);
    return list->last;
}
