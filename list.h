#ifndef SHAMON_LIST_H_
#define SHAMON_LIST_H_

#include <unistd.h>

/* non-circular doubly-linked-list */
typedef struct _shm_list_elem {
    struct _shm_list_elem *next, *prev;
    void *data;
} shm_list_elem;

typedef struct _shm_list {
    shm_list_elem *first;
    shm_list_elem *last;
    size_t size;
} shm_list;

typedef void (shm_list_elem_destroy_fn)(void *);

void shm_list_init(shm_list *list);
void shm_list_destroy(shm_list *list, shm_list_elem_destroy_fn destroy);

size_t shm_list_prepend(shm_list *list, void *elem);
size_t shm_list_append(shm_list *list, void *elem);
size_t shm_list_append_elem(shm_list *list, shm_list_elem *elem);
size_t shm_list_insert_after(shm_list *list, shm_list_elem *elem, void *data);
size_t shm_list_insert_before(shm_list *list, shm_list_elem *elem, void *data);
size_t shm_list_insert_elem_after(shm_list *list, shm_list_elem *elem, shm_list_elem *new);
size_t shm_list_remove(shm_list *list, shm_list_elem *elem);
size_t shm_list_size(shm_list *list);

shm_list_elem *shm_list_first(shm_list *list);
shm_list_elem *shm_list_last(shm_list *list);

#endif /* SHAMON_LIST_H_ */
