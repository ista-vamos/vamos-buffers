#include <stdio.h>
#include <sys/mman.h>

#include "buffer-private.h"
#include "shm.h"

#include "shmbuf/buffer.h"
#include "core/source.h"
#include "core/vector-macro.h"

char *get_sub_buffer_key(const char *key, size_t idx) {
    size_t tmpsize = strlen(key) + 16 /* space for index */;
    char *tmp = xalloc(tmpsize);
    int written = snprintf(tmp, tmpsize, "%s.sub.%lu", key, idx);
    if (written < 0 || written >= (int)tmpsize) {
        fprintf(stderr, "Failed creating sub-buffer key for '%s'\n", key);
        abort();
    }
    return tmp;
}

struct buffer *create_shared_sub_buffer(struct buffer *buffer, size_t capacity,
                                        const struct source_control *control) {
    char *key = get_sub_buffer_key(buffer->key, ++buffer->last_subbufer_no);
    struct source_control *ctrl =
        create_shared_control_buffer(key, S_IRWXU, control);
    if (!ctrl) {
        fprintf(stderr, "Failed creating control buffer\n");
        return NULL;
    }

    size_t elem_size = source_control_max_event_size(ctrl);
    if (capacity == 0)
        capacity = buffer_capacity(buffer);
    struct buffer *sbuf =
        initialize_shared_buffer(key, S_IRWXU, elem_size, capacity, ctrl);
    /* XXX: we copy the key in 'initialize_shared_buffer' which is redundant as
     * we have created it in `get_sub_buffer_key` and can just move it */
    free(key);

    ++buffer->shmbuffer->info.subbuffers_no;

    return sbuf;
}

size_t buffer_get_sub_buffers_no(struct buffer *buffer) {
    return buffer->shmbuffer->info.subbuffers_no;
}

/* for writers */
void destroy_shared_sub_buffer(struct buffer *buff) {
    buff->shmbuffer->info.destroyed = 1;

    size_t vecsize = VEC_SIZE(buff->aux_buffers);
    for (size_t i = 0; i < vecsize; ++i) {
        struct aux_buffer *ab = buff->aux_buffers[i];
        // we must first wait until the monitor finishes
        // aux_buffer_destroy(ab);
        aux_buffer_release(ab);
    }
    VEC_DESTROY(buff->aux_buffers);
    fprintf(stderr, "Totally used %lu aux buffers\n", vecsize);

    if (munmap(buff->shmbuffer, buff->shmbuffer->info.allocated_size) != 0) {
        perror("destroy_shared_buffer: munmap failure");
    }
    if (close(buff->fd) == -1) {
        perror("destroy_shared_buffer: failed closing mmap fd");
    }

    release_shared_control_buffer(buff->control);

    free(buff->key);
    free(buff);
}

/* for readers */
void release_shared_sub_buffer(struct buffer *buff) {
    if (munmap(buff->shmbuffer, buff->shmbuffer->info.allocated_size) != 0) {
        perror("release_shared_sub_buffer: munmap failure");
    }
    if (close(buff->fd) == -1) {
        perror("release_shared_sub_buffer: failed closing mmap fd");
    }

    size_t vecsize = VEC_SIZE(buff->aux_buffers);
    for (size_t i = 0; i < vecsize; ++i) {
        struct aux_buffer *ab = buff->aux_buffers[i];
        aux_buffer_release(ab);
    }
    VEC_DESTROY(buff->aux_buffers);

    if (shamon_shm_unlink(buff->key) != 0) {
        perror("release_shared_sub_buffer: shm_unlink failure");
    }

    destroy_shared_control_buffer(buff->key, buff->control);

    free(buff->key);
    free(buff);
}
