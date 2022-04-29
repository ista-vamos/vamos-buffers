#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "shm.h"
#include "buffer.h"

#define SLEEP_TIME_NS 10000
#define MEM_SIZE (128*4096)

struct buffer_info {
    size_t capacity;
    _Atomic size_t elem_num;
    size_t elem_size;
    size_t head;
    size_t tail;
    /* the monitored program exited/destroyed the buffer */
    _Bool destroyed;
    _Bool monitor_attached;
    /* number of dropped events */
    size_t dropped;
    /* shm filedescriptor */
    /* TODO -- keep this somewhere else, it is different between
     * processes */
    int fd;
} __attribute__((aligned(8)));

struct buffer {
    struct buffer_info __attribute__((aligned(8))) info;
    /* pointer to the beginning of data */
    unsigned char data[MEM_SIZE];
};

#define BUFF_START(b) ((void*)b->data)
#define BUFF_END(b) ((void*)b->data + MEM_SIZE - 1)

size_t buffer_allocation_size() {
    return sizeof(struct buffer);
}

bool buffer_is_ready(struct buffer *buff)
{
    return !buff->info.destroyed;
}

bool buffer_monitor_attached(struct buffer *buff)
{
    return buff->info.monitor_attached;
}

size_t buffer_capacity(struct buffer *buff)
{
    return buff->info.capacity;
}

size_t buffer_size(struct buffer *buff)
{
    return buff->info.elem_num;
}

size_t buffer_elem_size(struct buffer *buff)
{
    return buff->info.elem_size;
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

    struct buffer *buff = (struct buffer *)mem;
    memset(buff, 0, sizeof(struct buffer_info));
    buff->info.capacity = (BUFF_END(buff) - BUFF_START(buff)) / elem_size;
    printf("Buffer allocated size = %lu, capacity = %lu\n",
           buffer_allocation_size(), buff->info.capacity);
    buff->info.elem_size = elem_size;
    buff->info.fd = fd;

    return buff;
}

struct buffer *get_shared_buffer(const char *key)
{
    printf("Getting shared buffer '%s'\n", key);
    int fd = shamon_shm_open(key, O_RDWR|O_CREAT, S_IRWXU);
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

    struct buffer *buff = (struct buffer *)mem;

    return buff;
}

void buffer_set_attached(struct buffer *buff, bool val) {
    buff->info.monitor_attached = val;
}

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
    printf("   ... its size if %lu\n", size);

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
        perror("shm_unlink failure");
    }
}


void release_shared_buffer(struct buffer *buff)
{
    int fd = buff->info.fd;
    if (munmap(buff, sizeof(struct buffer)) != 0) {
        perror("munmap failure");
    }
    if (close(fd) == -1) {
        perror("closing fd after mmap failure");
    }
    if (shamon_shm_unlink(NULL) != 0) {
        perror("shm_unlink failure");
    }
}


void destroy_shared_buffer(struct buffer *buff)
{
    buff->info.destroyed = 1;
    release_shared_buffer(buff);
}

void *buffer_read_pointer(struct buffer *buff, size_t *size) {
    struct buffer_info *info = &buff->info;

    if (info->elem_num == 0) {
        *size = 0;
        return NULL;
    }

    unsigned char *pos = buff->data + info->tail*info->elem_size;
    size_t end = info->tail + *size;
    if (end > info->capacity) {
        *size -= end - info->capacity;
        assert(*size < info->elem_num);
    }

    return pos;
}

/* can be safely used only by the reader */
bool buffer_drop_k(struct buffer *buff, size_t k) {
     if (buff->info.elem_num >= k) {
         buff->info.tail += k;
         if (buff->info.tail >= buff->info.capacity)
             buff->info.tail -= buff->info.capacity;
         buff->info.elem_num -= k;
         return true;
     }
     return false;
}

bool buffer_push(struct buffer *buff, const void *elem, size_t size) {
    struct buffer_info *info = &buff->info;
    assert(!info->destroyed && "Writing to a destroyed buffer");
    /* buffer full */
    if (info->elem_num == info->capacity) {
        return false;
    }

    /* all ok, copy the data */
    assert(info->elem_size >= size && "Size does not fit the slot");
    void *pos = buff->data + info->head*info->elem_size;
    memcpy(pos, elem, size);
    ++info->head;

    // queue full, rotate it
    if (info->head == info->capacity) {
        info->head = 0;
    }

    // the increment must come after everything is done
    ++info->elem_num;

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
    struct buffer_info *info = &buff->info;
    assert(!info->destroyed && "Writing to a destroyed buffer");

    /* buffer full */
    if (info->elem_num == info->capacity) {
        /* ++info->dropped; */
        return NULL;
    }

    /* all ok, return the pointer to the data */
    /* FIXME: do not use multiplication, maintain the pointer to the head of data */
    return buff->data + info->head*info->elem_size;
}

void *buffer_partial_push(struct buffer *buff, void *prev_push,
                          const void *elem, size_t size) {
    assert(!buff->info.destroyed && "Writing to a destroyed buffer");
    /* buffer full */
    assert(buff->info.elem_num < buff->info.capacity);

    /* all ok, copy the data */
    assert(BUFF_START(buff) <= prev_push);
    assert(prev_push < BUFF_END(buff));
    assert(prev_push <= BUFF_END(buff) - size);
    memcpy(prev_push, elem, size);
    return prev_push + size;
}

bool buffer_finish_push(struct buffer *buff) {
    struct buffer_info *info = &buff->info;
    assert(!info->destroyed && "Writing to a destroyed buffer");
    /* buffer full */
    assert(info->elem_num != info->capacity);

    ++info->head;

    // queue full, rotate it
    if (info->head == info->capacity) {
        info->head = 0;
    }

    // the increment must come after everything is done
    ++info->elem_num;

    return true;
}

bool buffer_pop(struct buffer *buff, void *dst) {
    struct buffer_info *info = &buff->info;
    assert(!info->destroyed && "Reading from a destroyed buffer");
    if (info->elem_num == 0) {
        return false;
    }

    unsigned char *pos = buff->data + info->tail*info->elem_size;
    memcpy(dst, pos, info->elem_size);
    ++info->tail;
    if (info->tail == info->capacity)
        info->tail = 0;

    assert(info->elem_num > 0);
    --info->elem_num;

    return true;
}

/* copy k elements out of the buffer */
bool buffer_pop_k(struct buffer *buff, void *dst, size_t k) {
    struct buffer_info *info = &buff->info;
    assert(!info->destroyed && "Reading from a destroyed buffer");
    if (info->elem_num < k) {
        return false;
    }

    unsigned char *pos = buff->data + info->tail*info->elem_size;
    size_t end = info->tail + k;
    if (end > info->capacity) {
        memcpy(dst, pos, (info->capacity - info->tail)*info->elem_size);
        memcpy(dst + info->elem_size*(info->capacity - info->tail),
               buff->data, (end - info->capacity)*info->elem_size);
        info->tail = end - info->capacity;
    } else {
        memcpy(dst, pos, k*info->elem_size);
        info->tail = end;
    }

    assert(info->elem_num > 0);
    info->elem_num -= k;

    return true;
}


