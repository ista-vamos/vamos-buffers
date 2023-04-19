#ifndef SHAMON_LIST_EMBEDDED_H_
#define SHAMON_LIST_EMBEDDED_H_

#include <unistd.h>

/* circular embedded doubly-linked list */
typedef struct _vms_list_embedded {
    struct _vms_list_embedded *next;
    struct _vms_list_embedded *prev;
} vms_list_embedded;

void vms_list_embedded_init(vms_list_embedded *list);
void vms_list_embedded_insert_after(vms_list_embedded *list,
                                    vms_list_embedded *elm);
void vms_list_embedded_insert_list(vms_list_embedded *list,
                                   vms_list_embedded *other);
void vms_list_embedded_remove(vms_list_embedded *elm);
size_t vms_list_embedded_size(const vms_list_embedded *list);
_Bool vms_list_embedded_empty(const vms_list_embedded *list);

#define vms_list_embedded_object_of(ptr, tyobj, member) \
    (__typeof__(tyobj))((char *)(ptr)-offsetof(__typeof__(*tyobj), member))

#define vms_list_embedded_foreach(pos, head, member)                   \
    for (pos = vms_list_embedded_object_of((head)->next, pos, member); \
         &pos->member != (head);                                       \
         pos = vms_list_embedded_object_of(pos->member.next, pos, member))

#define vms_list_embedded_foreach_safe(pos, tmp, head, member)              \
    for (pos = vms_list_embedded_object_of((head)->next, pos, member),      \
        tmp = vms_list_embedded_object_of((pos)->member.next, tmp, member); \
         &pos->member != (head); pos = tmp,                                 \
        tmp = vms_list_embedded_object_of(pos->member.next, tmp, member))

#define vms_list_embedded_foreach_reverse(pos, head, member)           \
    for (pos = vms_list_embedded_object_of((head)->prev, pos, member); \
         &pos->member != (head);                                       \
         pos = vms_list_embedded_object_of(pos->member.prev, pos, member))

#define vms_list_embedded_foreach_reverse_safe(pos, tmp, head, member)      \
    for (pos = vms_list_embedded_object_of((head)->prev, pos, member),      \
        tmp = vms_list_embedded_object_of((pos)->member.prev, tmp, member); \
         &pos->member != (head); pos = tmp,                                 \
        tmp = vms_list_embedded_object_of(pos->member.prev, tmp, member))

#endif /* SHAMON_LIST_EMBEDDED_H_ */
