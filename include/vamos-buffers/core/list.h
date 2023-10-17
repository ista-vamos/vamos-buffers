#ifndef VAMOS_LIST_H_
#define VAMOS_LIST_H_

#include <unistd.h>

/* non-circular doubly-linked-list */
typedef struct _vms_list_elem {
    struct _vms_list_elem *next, *prev;
    void *data;
} vms_list_elem;

typedef struct _vms_list {
    vms_list_elem *first;
    vms_list_elem *last;
    size_t size;
} vms_list;

typedef void(vms_list_elem_destroy_fn)(void *);

void vms_list_init(vms_list *list);
void vms_list_destroy(vms_list *list, vms_list_elem_destroy_fn destroy);

size_t vms_list_prepend(vms_list *list, void *elem);
size_t vms_list_append(vms_list *list, void *elem);
size_t vms_list_append_elem(vms_list *list, vms_list_elem *elem);
size_t vms_list_insert_after(vms_list *list, vms_list_elem *elem, void *data);
size_t vms_list_insert_before(vms_list *list, vms_list_elem *elem, void *data);
size_t vms_list_insert_elem_after(vms_list *list, vms_list_elem *elem,
                                  vms_list_elem *new);
size_t vms_list_remove(vms_list *list, vms_list_elem *elem);
size_t vms_list_size(vms_list *list);

vms_list_elem *vms_list_first(vms_list *list);
vms_list_elem *vms_list_last(vms_list *list);

#endif /* VAMOS_LIST_H_ */
