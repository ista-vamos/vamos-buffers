#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <time.h>

#include "shm.h"
#include "buffer.h"
#include "list.h"
#include "vector-macro.h"

#define SLEEP_TIME_NS 10000
#define MEM_SIZE (1024*1024)

#define MAX_AUX_BUF_KEY_SIZE 16


struct buffer_info {
    size_t capacity;
    size_t elem_num;
    size_t elem_size;
    size_t head;
    size_t tail;
    /* the monitored program exited/destroyed the buffer */
    _Bool destroyed;
    _Bool monitor_attached;
} __attribute__((aligned(8)));

struct shmbuffer {
    struct buffer_info __attribute__((aligned(8))) info;
    /* pointer to the beginning of data */
    unsigned char data[MEM_SIZE];
};

struct aux_buffer {
    size_t size;
    size_t head;
    size_t idx;
    bool reusable;
    unsigned char data[0];
};


struct buffer {
    struct shmbuffer *shmbuffer;
    /* shared memory of auxiliary buffer */
    struct aux_buffer *cur_aux_buff;
    /* the known aux_buffers that might still be needed */
    VEC(aux_buffers, struct aux_buffer *);
    size_t aux_buf_idx;
    shm_list aux_buffers_age;
    /* shm filedescriptor */
    int fd;
    /* shm key */
    char *key;
};

static size_t aux_buffer_free_space(struct aux_buffer *buff);
static void aux_buffer_release(struct aux_buffer *buffer);
//static void aux_buffer_destroy(struct aux_buffer *buffer);
static size_t buffer_push_strn(struct buffer *buff, const void *data, size_t size);
static uint64_t buffer_push_str(struct buffer *buff, const char *str);

#define BUFF_START(b) ((void*)b->data)
#define BUFF_END(b) ((void*)b->data + MEM_SIZE - 1)

size_t buffer_allocation_size() {
    return sizeof(struct shmbuffer);
}

bool buffer_is_ready(struct buffer *buff)
{
    return !buff->shmbuffer->info.destroyed;
}

bool buffer_monitor_attached(struct buffer *buff)
{
    return buff->shmbuffer->info.monitor_attached;
}

size_t buffer_capacity(struct buffer *buff)
{
    return buff->shmbuffer->info.capacity;
}

size_t buffer_size(struct buffer *buff)
{
    return buff->shmbuffer->info.elem_num;
}

size_t buffer_elem_size(struct buffer *buff)
{
    return buff->shmbuffer->info.elem_size;
}

struct buffer *initialize_shared_buffer(size_t elem_size)
{
    assert(elem_size > 0 && "Element size is 0");
    const char *key = shamon_shm_default_key();
    printf("Initializing buffer '%s' with elem size '%lu'\n", key, elem_size);
    int fd = shamon_shm_open(key, O_RDWR|O_CREAT, S_IRWXU);
    if(fd < 0) {
        perror("shm_open");
        return NULL;
    }

    if((ftruncate(fd, buffer_allocation_size())) == -1) {
        perror("ftruncate");
        return NULL;
    }

    void *mem = mmap(0, buffer_allocation_size(),
                     PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        perror("mmap failure");
        if (close(fd) == -1) {
            perror("closing fd after mmap failure");
        }
        if (shamon_shm_unlink(key) != 0) {
            perror("shm_unlink after mmap failure");
        }
        return NULL;
    }

    struct buffer *buff = malloc(sizeof(struct buffer));
    assert(buff && "Memory allocation failed");

    buff->shmbuffer = (struct shmbuffer *)mem;
    memset(buff->shmbuffer, 0, sizeof(struct buffer_info));
    buff->shmbuffer->info.capacity
        = (BUFF_END(buff->shmbuffer) - BUFF_START(buff->shmbuffer)) / elem_size;
    printf("  .. buffer allocated size = %lu, capacity = %lu\n",
           buffer_allocation_size(), buff->shmbuffer->info.capacity);
    buff->shmbuffer->info.elem_size = elem_size;

    buff->key = strdup(key);
    VEC_INIT(buff->aux_buffers);
    shm_list_init(&buff->aux_buffers_age);
    buff->aux_buf_idx = 0;
    buff->cur_aux_buff = NULL;
    buff->fd = fd;

    puts("Done");
    return buff;
}

struct buffer *get_shared_buffer(const char *key)
{
    printf("Getting shared buffer '%s'\n", key);
    int fd = shamon_shm_open(key, O_RDWR, S_IRWXU);
    if(fd < 0) {
        perror("shm_open");
        return NULL;
    }

    void *mem = mmap(0, buffer_allocation_size(),
                     PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        perror("mmap failure");
        if (close(fd) == -1) {
            perror("closing fd after mmap failure");
        }
        if (shamon_shm_unlink(key) != 0) {
            perror("shm_unlink after mmap failure");
        }
        return NULL;
    }

    struct buffer *buff = malloc(sizeof(*buff));
    assert(buff && "Memory allocation failed");

    buff->shmbuffer = (struct shmbuffer *)mem;
    buff->key = strdup(key);
    VEC_INIT(buff->aux_buffers);
    buff->aux_buf_idx = 0;
    buff->cur_aux_buff = NULL;
    buff->fd = fd;

    return buff;
}

void buffer_set_attached(struct buffer *buff, bool val) {
    buff->shmbuffer->info.monitor_attached = val;
}

/* for readers */
void release_shared_buffer(struct buffer *buff)
{
    if (munmap(buff->shmbuffer, buffer_allocation_size()) != 0) {
        perror("release_shared_buffer: munmap failure");
    }
    if (close(buff->fd) == -1) {
        perror("release_shared_buffer: failed closing mmap fd");
    }
    free(buff->key);

    size_t vecsize = VEC_SIZE(buff->aux_buffers);
    for (size_t i = 0; i < vecsize; ++i) {
         struct aux_buffer *ab = buff->aux_buffers[i];
         aux_buffer_release(ab);
    }
    VEC_DESTROY(buff->aux_buffers);

    free(buff);
}

/* for writers */
void destroy_shared_buffer(struct buffer *buff)
{
    buff->shmbuffer->info.destroyed = 1;

    size_t vecsize = VEC_SIZE(buff->aux_buffers);
    for (size_t i = 0; i < vecsize; ++i) {
         struct aux_buffer *ab = buff->aux_buffers[i];
         // we must first wait until the monitor finishes
         //aux_buffer_destroy(ab);
         aux_buffer_release(ab);
    }
    VEC_DESTROY(buff->aux_buffers);

    if (munmap(buff->shmbuffer, buffer_allocation_size()) != 0) {
        perror("destroy_shared_buffer: munmap failure");
    }
    if (close(buff->fd) == -1) {
        perror("destroy_shared_buffer: failed closing mmap fd");
    }
    if (shamon_shm_unlink(buff->key) != 0) {
        perror("destroy_shared_buffer: shm_unlink failure");
    }

    free(buff->key);
    free(buff);
}

void *buffer_read_pointer(struct buffer *buff, size_t *size) {
    struct buffer_info *info = &buff->shmbuffer->info;

    if (info->elem_num == 0) {
        *size = 0;
        return NULL;
    }

    unsigned char *pos = buff->shmbuffer->data + info->tail*info->elem_size;
    size_t end = info->tail + *size;
    if (end > info->capacity) {
        *size -= end - info->capacity;
        assert(*size <= info->elem_num);
    }

    return pos;
}

/* can be safely used only by the reader */
bool buffer_drop_k(struct buffer *buff, size_t k) {
    struct buffer_info *info = &buff->shmbuffer->info;
     if (info->elem_num >= k) {
         info->tail += k;
         if (info->tail >= info->capacity)
             info->tail -= info->capacity;
         /* Do info->elem_num -= k atomically. */
         atomic_fetch_sub_explicit(&info->elem_num, k,
                                   memory_order_relaxed);

         return true;
     }
     return false;
}

bool buffer_push(struct buffer *buff, const void *elem, size_t size) {
    struct buffer_info *info = &buff->shmbuffer->info;
    assert(!info->destroyed && "Writing to a destroyed buffer");
    /* buffer full */
    if (info->elem_num == info->capacity) {
        return false;
    }

    /* all ok, copy the data */
    assert(info->elem_size >= size && "Size does not fit the slot");
    void *pos = buff->shmbuffer->data + info->head*info->elem_size;
    memcpy(pos, elem, size);
    ++info->head;

    // queue full, rotate it
    if (info->head == info->capacity) {
        info->head = 0;
    }

    /* Do ++info->elem_num atomically. */
    /* The increment must come after everything is done.
       The release order makes sure that the written element
       is visible to other threads by now. */
    atomic_fetch_add_explicit(&info->elem_num, 1,
                              memory_order_release);

    return true;
}

/* buffer_push broken down into several operations:
 *
 *  p = buffer_start_push(...)
 *  p = buffer_partial_push(..., p, ...)
 *  ...
 *  p = buffer_partial_push(..., p, ...)
 *  buffer_partial_push(..., p, ...)
 *  buffer_finish_push(...)
 *
 * All the partial push may push only a single element in sum,
 * i.e., what can be done with buffer_push. Partial pushes
 * cannot be mixed nor combined with normal push operations.
 * These are really just one buffer_push broken down into
 * multiple steps.
 */

void *buffer_start_push(struct buffer *buff) {
    struct buffer_info *info = &buff->shmbuffer->info;
    assert(!info->destroyed && "Writing to a destroyed buffer");

    /* buffer full */
    if (info->elem_num == info->capacity) {
        /* ++info->dropped; */
        return NULL;
    }

    /* all ok, return the pointer to the data */
    /* FIXME: do not use multiplication, maintain the pointer to the head of data */
    return buff->shmbuffer->data + info->head*info->elem_size;
}

void *buffer_partial_push(struct buffer *buff, void *prev_push,
                          const void *elem, size_t size) {
    assert(buffer_is_ready(buff) && "Writing to a destroyed buffer");
    /* buffer full */
    assert(buff->shmbuffer->info.elem_num < buff->shmbuffer->info.capacity);

    /* all ok, copy the data */
    assert(BUFF_START(buff->shmbuffer) <= prev_push);
    assert(prev_push < BUFF_END(buff->shmbuffer));
    assert(prev_push <= BUFF_END(buff->shmbuffer) - size);
    memcpy(prev_push, elem, size);
    return prev_push + size;
}

void *buffer_partial_push_str(struct buffer *buff, void *prev_push,
                              const char *str) {
    assert(!buff->shmbuffer->info.destroyed && "Writing to a destroyed buffer");
    /* buffer full */
    assert(buff->shmbuffer->info.elem_num < buff->shmbuffer->info.capacity);

    /* all ok, copy the data */
    assert(BUFF_START(buff->shmbuffer) <= prev_push);
    assert(prev_push < BUFF_END(buff->shmbuffer));

    *((uint64_t *)prev_push) = buffer_push_str(buff, str);
    /*printf("Pushed str: %lu\n", *((uint64_t *)prev_push));*/
    return prev_push + sizeof(uint64_t);
}


bool buffer_finish_push(struct buffer *buff) {
    struct buffer_info *info = &buff->shmbuffer->info;
    assert(!info->destroyed && "Writing to a destroyed buffer");
    /* buffer full */
    assert(info->elem_num != info->capacity);

    ++info->head;

    // queue full, rotate it
    if (info->head == info->capacity) {
        info->head = 0;
    }

    /* Do ++info->elem_num atomically. */
    /* The increment must come after everything is done.
       The release order makes sure that the written element
       is visible to other threads by now. */
    atomic_fetch_add_explicit(&info->elem_num, 1,
                              memory_order_release);

    return true;
}

bool buffer_pop(struct buffer *buff, void *dst) {
    struct buffer_info *info = &buff->shmbuffer->info;
    assert(!info->destroyed && "Reading from a destroyed buffer");
    if (info->elem_num == 0) {
        return false;
    }

    unsigned char *pos = buff->shmbuffer->data + info->tail*info->elem_size;
    memcpy(dst, pos, info->elem_size);
    ++info->tail;
    if (info->tail == info->capacity)
        info->tail = 0;

    assert(info->elem_num > 0);
    /* Do  --info->elem_num atomically */
    atomic_fetch_sub_explicit(&info->elem_num, 1,
                              memory_order_relaxed);

    return true;
}

/* copy k elements out of the buffer */
bool buffer_pop_k(struct buffer *buff, void *dst, size_t k) {
    struct buffer_info *info = &buff->shmbuffer->info;
    assert(!info->destroyed && "Reading from a destroyed buffer");
    if (info->elem_num < k) {
        return false;
    }

    unsigned char *pos = buff->shmbuffer->data + info->tail*info->elem_size;
    size_t end = info->tail + k;
    if (end > info->capacity) {
        memcpy(dst, pos, (info->capacity - info->tail)*info->elem_size);
        memcpy(dst + info->elem_size*(info->capacity - info->tail),
               buff->shmbuffer->data, (end - info->capacity)*info->elem_size);
        info->tail = end - info->capacity;
    } else {
        memcpy(dst, pos, k*info->elem_size);
        info->tail = end;
    }

    assert(info->elem_num > 0);
    /* Do info->elem_num -= k atomically.
     * The decrement must come after everything is done.
       The order may be 'relaxed' as we have only one reader
       and thus the writes to shared variables will be
       up to date for this thread. */
    atomic_fetch_sub_explicit(&info->elem_num, k,
                              memory_order_relaxed);

    return true;
}

/*** CONTROL BUFFER ****/

void *initialize_shared_control_buffer(size_t size)
{
    const char *key = shamon_shm_default_ctrl_key();
    printf("Initializing control buffer '%s' of size '%lu'\n", key, size);
    int fd = shamon_shm_open(key, O_RDWR|O_CREAT, S_IRWXU);
    if(fd < 0) {
        perror("shm_open");
        return NULL;
    }

    if((ftruncate(fd, size + 2*sizeof(size_t))) == -1) {
        perror("ftruncate");
        return NULL;
    }

    void *mem = mmap(0, size + 2*sizeof(size_t),
                     PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        perror("mmap failure");
        if (close(fd) == -1) {
            perror("closing fd after mmap failure");
        }
        if (shamon_shm_unlink(key) != 0) {
            perror("shm_unlink after mmap failure");
        }
        return NULL;
    }

    memset(mem, 0, size);
    size_t *smem = (size_t *)mem;
    *smem++ = size;
    *smem++ = (size_t) fd;

    assert(control_buffer_size((void*)smem) == size);
    return (void*) smem;
}

void *get_shared_control_buffer()
{
    const char *key = shamon_shm_default_ctrl_key();
    printf("Getting control buffer '%s'\n", key);
    int fd = shamon_shm_open(key, O_RDWR|O_CREAT, S_IRWXU);
    if(fd < 0) {
        perror("shm_open");
        return NULL;
    }

    size_t size;
    if (read(fd, &size, sizeof(size)) == -1) {
        perror("reading size of ctrl buffer");
        return NULL;
    }
    printf("   ... its size is %lu\n", size);

    void *mem = mmap(0, size + 2*sizeof(size_t),
                     PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        perror("mmap failure");
        if (close(fd) == -1) {
            perror("closing fd after mmap failure");
        }
        if (shamon_shm_unlink(key) != 0) {
            perror("shm_unlink after mmap failure");
        }
        return NULL;
    }

    /* FIXME: we leak fd */
    return mem + 2*sizeof(size_t);
}

size_t control_buffer_size(void *buffer)
{
    size_t *mem = (size_t *) buffer;
    return *(mem - 2);
}

void release_shared_control_buffer(void *buffer)
{
    size_t *mem = (size_t *) buffer;
    int fd = (int)*(--mem);
    size_t size = *(--mem);

    if (munmap(mem, size) != 0) {
        perror("munmap failure");
    }
    if (close(fd) == -1) {
        perror("closing fd after mmap failure");
    }
    if (shamon_shm_unlink(shamon_shm_default_ctrl_key()) != 0) {
        perror("release_shared_control_buffer: shm_unlink failure");
    }
}

/***** AUX BUFFERS ********/
size_t aux_buffer_free_space(struct aux_buffer *buff) {
    return buff->size - buff->head;
}

static struct aux_buffer *new_aux_buffer(struct buffer *buff, size_t size) {
    size_t idx = buff->aux_buf_idx++;
    const size_t pg_size = getpagesize();
    size = (((size + sizeof(struct aux_buffer)) / pg_size)+9)*pg_size;

    /* create the key */
    char key[20];
    snprintf(key, 19, "/aux.%lu", idx);

    printf("Initializing aux buffer %s\n", key);

    int fd = shamon_shm_open(key, O_RDWR|O_CREAT, S_IRWXU);
    if(fd < 0) {
        perror("shm_open");
        abort();
    }

    if((ftruncate(fd, size)) == -1) {
        perror("ftruncate");
        abort();
    }

    void *mem = mmap(0, size,
                     PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
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

    struct aux_buffer *ab = (struct aux_buffer*) mem;
    ab->head = 0;
    ab->size = size - sizeof(struct aux_buffer);
    ab->idx = idx;
    ab->reusable = false;

    VEC_PUSH(buff->aux_buffers, &ab);
    assert(VEC_TOP(buff->aux_buffers) == ab);
    shm_list_append(&buff->aux_buffers_age, ab);
    assert(shm_list_last(&buff->aux_buffers_age)->data == ab);
    buff->cur_aux_buff = ab;

    return ab;
}

static struct aux_buffer *writer_get_aux_buffer(struct buffer *buff, size_t size) {
    if (!buff->cur_aux_buff || aux_buffer_free_space(buff->cur_aux_buff) < size) {
        /* try to find a free buffer */
        struct aux_buffer *ab;
        shm_list_elem *cur = shm_list_first(&buff->aux_buffers_age);
        while (cur) {
            ab = (struct aux_buffer *) cur->data;
            if (ab->reusable && ab->size >= size) {
                assert(shm_list_last(&buff->aux_buffers_age)->data == buff->cur_aux_buff);
                shm_list_remove(&buff->aux_buffers_age, cur);
                shm_list_append_elem(&buff->aux_buffers_age, cur);
                buff->cur_aux_buff = ab;
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

static struct aux_buffer *reader_get_aux_buffer(struct buffer *buff, size_t idx) {
    /* cache the last use */
    if (buff->cur_aux_buff && buff->cur_aux_buff->idx == idx)
        return buff->cur_aux_buff;

    /* try to find one with the idx */
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

    //printf("Getting aux buffer %s\n", key);

    int fd = shamon_shm_open(key, O_RDWR, S_IRWXU);
    if(fd < 0) {
        perror("shm_open");
        abort();
    }

    size_t size;
    if (read(fd, &size, sizeof(size)) == -1) {
        perror("reading size of aux buffer");
        return NULL;
    }
    //printf("   ... its size is %lu\n", size);

    void *mem = mmap(0, size + 3*sizeof(size_t),
                     PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
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

    struct aux_buffer *ab = (struct aux_buffer*)mem;
    assert(ab->idx == idx && "Got wrong buffer");
    assert(ab->size > 0);

    buff->cur_aux_buff = ab;
    VEC_PUSH(buff->aux_buffers, &ab);
    assert(VEC_TOP(buff->aux_buffers) == ab);

    return ab;
}

void aux_buffer_release(struct aux_buffer *buffer)
{
    assert((buffer->size + sizeof(struct aux_buffer)) % getpagesize() == 0);
    //int fd = buffer->fd;
    if (munmap(buffer, buffer->size + sizeof(struct aux_buffer)) != 0) {
        perror("aux_buffer_destroy: munmap failure");
    }
    /* FIXME
    if (close(fd) == -1) {
        perror("aux_buffer_destroy: closing fd after mmap failure");
    }
    */
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


size_t buffer_push_strn(struct buffer *buff,
                        const void *data, size_t size)
{
    struct aux_buffer *ab = writer_get_aux_buffer(buff, size);
    assert(ab);
    assert(ab == buff->cur_aux_buff);
    assert(shm_list_last(&buff->aux_buffers_age)->data == buff->cur_aux_buff);

    size_t off = ab->head;
    assert(off < (1LU<<32));

    memcpy(ab->data + off, data, size);
    ab->head += size;
    return off;
}

void *buffer_get_str(struct buffer *buff, uint64_t elem)
{
    size_t idx = elem >> 32;
    struct aux_buffer *ab = reader_get_aux_buffer(buff, idx);
    size_t off = elem & 0xffffffff;
    return ab->data + off;
}

uint64_t buffer_push_str(struct buffer *buff, const char *str) {
    size_t len = strlen(str) + 1;
    size_t off = buffer_push_strn(buff, str, len);
    assert(buff->cur_aux_buff);
    return (off | (buff->cur_aux_buff->idx << 32));
}
