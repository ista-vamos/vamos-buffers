#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>

#include "buffer-private.h"
#include "shm.h"

HIDE_SYMBOL
void aux_buffer_release(struct aux_buffer *buffer) {
    assert((buffer->size + sizeof(struct aux_buffer)) % sysconf(_SC_PAGESIZE) ==
           0);
    // int fd = buffer->fd;
    if (munmap(buffer, buffer->size + sizeof(struct aux_buffer)) != 0) {
        perror("aux_buffer_destroy: munmap failure");
    }
    /* FIXME
    if (close(fd) == -1) {
        perror("aux_buffer_destroy: closing fd after mmap failure");
    }
    */
}

HIDE_SYMBOL
size_t aux_buffer_free_space(struct aux_buffer *buff) {
    return buff->size - buff->head;
}

/*
void aux_buffer_destroy(struct aux_buffer *buffer) {
    char key[20];
    snprintf(key, 19, "/aux.%lu", buffer->idx);

    aux_buffer_release(buffer);

    if (shamon_shm_unlink(key) != 0) {
        perror("aux_buffer_destroy: shm_unlink failure");
    }
}
*/

static struct aux_buffer *new_aux_buffer(struct buffer *buff, size_t size) {
    size_t idx = buff->aux_buf_idx++;
    const size_t pg_size = sysconf(_SC_PAGESIZE);
    size = (((size + sizeof(struct aux_buffer)) / pg_size) + 2) * pg_size;

    /* create the key */
    char key[20];
    snprintf(key, 19, "/aux.%lu", idx);

    /* printf("Initializing aux buffer %s\n", key); */

    int fd = shamon_shm_open(key, O_RDWR | O_CREAT, buff->mode);
    if (fd < 0) {
        perror("shm_open");
        abort();
    }

    if ((ftruncate(fd, size)) == -1) {
        perror("ftruncate");
        abort();
    }

    void *mem = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        perror("mmap failure");
        if (close(fd) == -1) {
            perror("closing fd after mmap failure");
        }
        if (shamon_shm_unlink(key) != 0) {
            perror("shm_unlink after mmap failure");
        }
        abort();
    }

    struct aux_buffer *ab = (struct aux_buffer *)mem;
    ab->head = 0;
    ab->size = size - sizeof(struct aux_buffer);
    ab->idx = idx;
    ab->first_event_id = 0;
    ab->last_event_id = ~(0LL);
    ab->reusable = false;

    VEC_PUSH(buff->aux_buffers, &ab);
    assert(VEC_TOP(buff->aux_buffers) == ab);
    shm_list_append(&buff->aux_buffers_age, ab);
    assert(shm_list_last(&buff->aux_buffers_age)->data == ab);
    buff->cur_aux_buff = ab;

    return ab;
}

static inline bool ab_was_dropped(struct aux_buffer *ab, struct buffer *buff) {
    struct buffer_info *info = &buff->shmbuffer->info;
    drop_ranges_lock(buff);
    for (size_t i = 0; i < DROPPED_RANGES_NUM; ++i) {
        struct dropped_range *r = &info->dropped_ranges[i];
        if (r->end == 0)
            continue;
        if (r->begin <= ab->first_event_id && r->end >= ab->last_event_id) {
            drop_ranges_unlock(buff);
            return true;
        }
    }
    drop_ranges_unlock(buff);

    return false;
}

HIDE_SYMBOL
struct aux_buffer *writer_get_aux_buffer(struct buffer *buff, size_t size) {
    if (!buff->cur_aux_buff ||
        aux_buffer_free_space(buff->cur_aux_buff) < size) {
        /* try to find a free buffer */
        struct aux_buffer *ab;
        shm_list_elem *cur = shm_list_first(&buff->aux_buffers_age);
        while (cur) {
            ab = (struct aux_buffer *)cur->data;
            if (ab->last_event_id <= buff->shmbuffer->info.last_processed_id ||
                ab_was_dropped(ab, buff)) {
                ab->reusable = true;
                ab->head = 0;
                ab->first_event_id = 0;
                ab->last_event_id = ~(0LL);
            }
            if (ab->reusable && ab->size >= size) {
                assert(shm_list_last(&buff->aux_buffers_age)->data ==
                       buff->cur_aux_buff);
                shm_list_remove(&buff->aux_buffers_age, cur);
                shm_list_append_elem(&buff->aux_buffers_age, cur);
                buff->cur_aux_buff = ab;
                ab->reusable = false;
                return ab;
            }
            cur = cur->next;
        }

        ab = new_aux_buffer(buff, size);
        return ab;
    }

    /* use the current buffer */
    /* it must always be the last in the aux_buffers_age */
    assert(shm_list_last(&buff->aux_buffers_age)->data == buff->cur_aux_buff);
    return buff->cur_aux_buff;
}

HIDE_SYMBOL
struct aux_buffer *reader_get_aux_buffer(struct buffer *buff, size_t idx) {
    /* cache the last use */
    if (buff->cur_aux_buff && buff->cur_aux_buff->idx == idx)
        return buff->cur_aux_buff;

    /* try to find one with the idx */
    /* FIXME: make it constant access */
    for (size_t i = 0; i < VEC_SIZE(buff->aux_buffers); ++i) {
        struct aux_buffer *ab = buff->aux_buffers[i];
        if (ab->idx == idx) {
            buff->cur_aux_buff = ab;
            return ab;
        }
    }

    /* create the key */
    char key[20];
    snprintf(key, 19, "/aux.%lu", idx);

    // printf("Getting aux buffer %s\n", key);

    int fd = shamon_shm_open(key, O_RDWR, S_IRWXU);
    if (fd < 0) {
        perror("shm_open");
        abort();
    }

    size_t size;
    if (read(fd, &size, sizeof(size)) == -1) {
        perror("reading size of aux buffer");
        return NULL;
    }
    // printf("   ... its size is %lu\n", size);

    void *mem = mmap(0, size + 3 * sizeof(size_t), PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        perror("mmap failure");
        if (close(fd) == -1) {
            perror("closing fd after mmap failure");
        }
        if (shamon_shm_unlink(key) != 0) {
            perror("shm_unlink after mmap failure");
        }
        abort();
    }

    struct aux_buffer *ab = (struct aux_buffer *)mem;
    assert(ab->idx == idx && "Got wrong buffer");
    assert(ab->size > 0);

    buff->cur_aux_buff = ab;
    VEC_PUSH(buff->aux_buffers, &ab);
    assert(VEC_TOP(buff->aux_buffers) == ab);

    return ab;
}
