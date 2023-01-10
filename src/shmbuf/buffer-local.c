#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "buffer-private.h"
#include "shmbuf/buffer.h"
#include "shamon/core/list.h"
#include "shamon/core/source.h"
#include "shamon/core/spsc_ringbuf.h"
#include "shamon/core/utils.h"
#include "shamon/core/vector-macro.h"

/* FOR TESTING */
struct buffer *initialize_local_buffer(const char *key, size_t elem_size,
                                       size_t capacity,
                                       struct source_control *control) {
    assert(elem_size > 0 && "Element size is 0");
    printf("Initializing LOCAL buffer '%s' with elem size '%lu'\n", key,
           elem_size);
    void *mem;
    const size_t memsize = compute_shm_size(elem_size, capacity);
    int succ = posix_memalign(&mem, 64, memsize);
    if (succ != 0) {
        perror("allocation failure");
        return NULL;
    }

    struct buffer *buff = malloc(sizeof(struct buffer));
    assert(buff && "Memory allocation failed");
    buff->shmbuffer = (struct shmbuffer *)mem;

    assert(ADDR_IS_CACHE_ALIGNED(buff->shmbuffer->data));
    assert(ADDR_IS_CACHE_ALIGNED(&buff->shmbuffer->info.ringbuf));

    memset(buff->shmbuffer, 0, sizeof(struct buffer_info));

    /* ringbuf has one dummy element */
    buff->shmbuffer->info.capacity = capacity;
    buff->shmbuffer->info.allocated_size = memsize;
    shm_spsc_ringbuf_init(_ringbuf(buff), capacity + 1);
    printf("  .. buffer allocated size = %lu, capacity = %lu\n",
           buff->shmbuffer->info.allocated_size,
           buff->shmbuffer->info.capacity);
    buff->shmbuffer->info.elem_size = elem_size;
    buff->shmbuffer->info.last_processed_id = 0;
    buff->shmbuffer->info.dropped_ranges_next = 0;
    buff->shmbuffer->info.dropped_ranges_lock = false;

    buff->key = strdup(key);
    VEC_INIT(buff->aux_buffers);
    shm_list_init(&buff->aux_buffers_age);
    buff->aux_buf_idx = 0;
    buff->cur_aux_buff = NULL;
    buff->fd = -1;
    buff->control = control;

    puts("Done");
    return buff;
}

void release_local_buffer(struct buffer *buff) {
    free(buff->key);

    size_t vecsize = VEC_SIZE(buff->aux_buffers);
    for (size_t i = 0; i < vecsize; ++i) {
        struct aux_buffer *ab = buff->aux_buffers[i];
        aux_buffer_release(ab);
    }
    VEC_DESTROY(buff->aux_buffers);

    free(buff);
}
