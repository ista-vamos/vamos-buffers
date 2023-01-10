#include <assert.h>
#include <stdio.h>

#include "buffer-private.h"

size_t compute_shm_size(size_t elem_size, size_t capacity) {
    /* compute how much memory we need */
    size_t size = (elem_size * capacity) + sizeof(struct shmbuffer);
    /* round it up to page size.
     * XXX: do we need that? mmap will do this internally anyway */
    size_t roundup = PAGE_SIZE - (size % PAGE_SIZE);
    if (roundup > (PAGE_SIZE) / 4) {
        fprintf(stderr,
                "The required capacity '%lu' of SHM buffer will result in %lu "
                "unused bytes "
                "in a memory page, consider changing it.\n"
                "You have space for %lu more elements...\n",
                capacity - 1, roundup, (roundup / elem_size));
    }
    size += roundup;
    assert((size % PAGE_SIZE) == 0);
    return size;
}
