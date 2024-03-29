#include <assert.h>
#include <stdio.h>

#include "buffer-private.h"

size_t compute_vms_size(size_t elem_size, size_t capacity) {
    /* compute how much memory we need */
    size_t size = (elem_size * capacity) + sizeof(struct shmbuffer);
    /* round it up to page size.
     * XXX: do we need that? mmap will do this internally anyway */
    size_t roundup = PAGE_SIZE - (size % PAGE_SIZE);
#ifndef NDEBUG
    if (roundup > (PAGE_SIZE) / 4) {
        fprintf(stderr,
                "[vamos] the required capacity '%lu' of SHM buffer will result "
                "in %lu "
                "unused bytes in a memory page, consider changing it. "
                "You have space for %lu more elements...\n",
                capacity - 1, roundup, (roundup / elem_size));
    }
#endif
    size += roundup;
    assert((size % PAGE_SIZE) == 0);
    return size;
}

size_t compute_vms_buffer_size(size_t nondata_size, size_t elem_size,
                               size_t capacity) {
    /* compute how much memory we need */
    size_t size = (elem_size * capacity) + nondata_size;
    size_t roundup = PAGE_SIZE - (size % PAGE_SIZE);
#ifndef NDEBUG
    if (roundup > (PAGE_SIZE) / 4) {
        fprintf(stderr,
                "[vamos] the required capacity '%lu' of a DBG buffer will "
                "result in %lu "
                "unused bytes in a memory page, consider changing it. "
                "You have space for %lu more elements...\n",
                capacity, roundup, (roundup / elem_size));
    }
#endif
    size += roundup;
    assert((size % PAGE_SIZE) == 0);
    return size;
}
