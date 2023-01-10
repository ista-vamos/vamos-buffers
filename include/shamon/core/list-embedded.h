#ifndef SHAMON_LIST_EMBEDDED_H_
#define SHAMON_LIST_EMBEDDED_H_

#include <unistd.h>

/* circular embedded doubly-linked list */
typedef struct _shm_list_embedded {
    struct _shm_list_embedded *next;
    struct _shm_list_embedded *prev;
} shm_list_embedded;

void shm_list_embedded_init(shm_list_embedded *list);
void shm_list_embedded_insert_after(shm_list_embedded *list,
                                    shm_list_embedded *elm);
void shm_list_embedded_insert_list(shm_list_embedded *list,
                                   shm_list_embedded *other);
void shm_list_embedded_remove(shm_list_embedded *elm);
size_t shm_list_embedded_size(const shm_list_embedded *list);
_Bool shm_list_embedded_empty(const shm_list_embedded *list);

#define shm_list_embedded_object_of(ptr, tyobj, member) \
    (__typeof__(tyobj))((char *)(ptr)-offsetof(__typeof__(*tyobj), member))

#define shm_list_embedded_foreach(pos, head, member)                   \
    for (pos = shm_list_embedded_object_of((head)->next, pos, member); \
         &pos->member != (head);                                       \
         pos = shm_list_embedded_object_of(pos->member.next, pos, member))

#define shm_list_embedded_foreach_safe(pos, tmp, head, member)              \
    for (pos = shm_list_embedded_object_of((head)->next, pos, member),      \
        tmp = shm_list_embedded_object_of((pos)->member.next, tmp, member); \
         &pos->member != (head); pos = tmp,                                 \
        tmp = shm_list_embedded_object_of(pos->member.next, tmp, member))

#define shm_list_embedded_foreach_reverse(pos, head, member)           \
    for (pos = shm_list_embedded_object_of((head)->prev, pos, member); \
         &pos->member != (head);                                       \
         pos = shm_list_embedded_object_of(pos->member.prev, pos, member))

#define shm_list_embedded_foreach_reverse_safe(pos, tmp, head, member)      \
    for (pos = shm_list_embedded_object_of((head)->prev, pos, member),      \
        tmp = shm_list_embedded_object_of((pos)->member.prev, tmp, member); \
         &pos->member != (head); pos = tmp,                                 \
        tmp = shm_list_embedded_object_of(pos->member.prev, tmp, member))

#endif /* SHAMON_LIST_EMBEDDED_H_ */
