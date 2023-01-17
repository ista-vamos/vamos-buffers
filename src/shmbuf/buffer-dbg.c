#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>

#include "buffer-private.h"
#include "shm.h"

/**
 * @brief The __shm_dbg_buffer class
 *
 * vms_shm_dbg_buffer is a versioned array of key+value records
 */
struct __shm_dbg_buffer {
    CACHELINE_ALIGNED struct {
        /* allocation size of the buffer */
        size_t allocation_size;
        /* maximal number of records in the buffer */
        size_t capacity;
        /* current number of elements in the buffer */
        _Atomic size_t size;
        /* size of key and element */
        uint16_t key_size;
        uint16_t value_size;
        /* the version will increase with every update, so that the reader can
         * keep consistent cache */
        volatile _Atomic size_t version;
    } info;

    /* here the data start */
    unsigned char data[];
};

typedef struct _vms_shm_dbg_buffer {
    /* shared-memory buffer */
    struct __shm_dbg_buffer *buffer;

    /* mmap fd, key, etc. */
    int fd;
    char *key;
    /* data pointer */
    unsigned char *data; /* = buffer->data */
} vms_shm_dbg_buffer;

static void dbg_buffer_init(struct __shm_dbg_buffer *b, size_t allocation_size,
                            size_t capacity, uint16_t key_size,
                            uint16_t value_size) {
    b->info.allocation_size = allocation_size;
    b->info.key_size = key_size;
    b->info.value_size = value_size;
    b->info.capacity = capacity;
    b->info.size = 0;
    b->info.version = 0;
}

vms_shm_dbg_buffer *vms_shm_dbg_buffer_create(const char *key, size_t capacity,
                                              uint16_t key_size,
                                              uint16_t value_size) {
    printf("Initializing dbg buffer '%s' of capacity '%lu'\n", key, capacity);

    char tmpkey[SHM_NAME_MAXLEN] = "";
    if (shamon_get_tmp_key(key, tmpkey, SHM_NAME_MAXLEN) == -1) {
        fprintf(stderr, "Failed creating a tmpkey for '%s'\n", key);
        return NULL;
    }

    int fd = shamon_shm_open(tmpkey, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        perror("shm_open");
        return NULL;
    }

    size_t allocation_size = compute_shm_buffer_size(
        sizeof(struct __shm_dbg_buffer), key_size + value_size, capacity);
    assert(allocation_size > sizeof(struct __shm_dbg_buffer));

    if ((ftruncate(fd, allocation_size)) == -1) {
        perror("ftruncate");
        return NULL;
    }

    void *mem =
        mmap(0, allocation_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
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

    vms_shm_dbg_buffer *buff = (vms_shm_dbg_buffer *)xalloc(sizeof(*buff));
    buff->buffer = (struct __shm_dbg_buffer *)mem;
    dbg_buffer_init(buff->buffer, allocation_size, capacity, key_size,
                    value_size);
    buff->data = buff->buffer->data;
    buff->key = xstrdup(key);
    buff->fd = fd;

    if (shamon_shm_rename(tmpkey, key) < 0) {
        perror("renaming SHM file");

        free(buff);

        if (close(fd) == -1) {
            perror("closing fd after mmap failure");
        }
        if (shamon_shm_unlink(tmpkey) != 0) {
            perror("shm_unlink after mmap failure");
        }
        if (munmap(mem, allocation_size) != 0) {
            perror("munmap of control buffer");
        }
        return NULL;
    }

    return buff;
}

vms_shm_dbg_buffer *vms_shm_dbg_buffer_get(const char *key) {
    printf("Getting dbg buffer '%s'\n", key);

    int fd = shamon_shm_open(key, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        perror("shm_open");
        return NULL;
    }

    struct __shm_dbg_buffer tmp;
    if (pread(fd, &tmp, sizeof(tmp), 0) == -1) {
        perror("reading info about dbg buffer");
        return NULL;
    }
    assert(tmp.info.allocation_size > 0);

    if ((ftruncate(fd, tmp.info.allocation_size)) == -1) {
        perror("ftruncate");
        return NULL;
    }

    void *mem = mmap(0, tmp.info.allocation_size, PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd, 0);
    if (mem == MAP_FAILED) {
        perror("mmap failure");
        if (close(fd) == -1) {
            perror("closing fd after mmap failure");
        }
        return NULL;
    }

    vms_shm_dbg_buffer *buff = (vms_shm_dbg_buffer *)xalloc(sizeof(*buff));
    buff->buffer = (struct __shm_dbg_buffer *)mem;
    buff->data = buff->buffer->data;
    buff->key = xstrdup(key);
    buff->fd = fd;

    return buff;
}

void vms_shm_dbg_buffer_release(vms_shm_dbg_buffer *buff) {
    if (close(buff->fd) == -1) {
        perror("closing fd when releasing dbg buffer");
    }
    if (munmap(buff->buffer, buff->buffer->info.allocation_size) != 0) {
        perror("munmap of dbg buffer");
    }

    free(buff->key);
    free(buff);
}

void vms_shm_dbg_buffer_destroy(vms_shm_dbg_buffer *buff) {
    if (shamon_shm_unlink(buff->key) != 0) {
        perror("shm_unlink when destroying a dbg buffer");
    }

    vms_shm_dbg_buffer_release(buff);
}

size_t vms_shm_dbg_buffer_size(vms_shm_dbg_buffer *b) {
    return b->buffer->info.size;
}
size_t vms_shm_dbg_buffer_capacity(vms_shm_dbg_buffer *b) {
    return b->buffer->info.capacity;
}
size_t vms_shm_dbg_buffer_key_size(vms_shm_dbg_buffer *b) {
    return b->buffer->info.key_size;
}
size_t vms_shm_dbg_buffer_value_size(vms_shm_dbg_buffer *b) {
    return b->buffer->info.value_size;
}

size_t vms_shm_dbg_buffer_rec_size(vms_shm_dbg_buffer *b) {
    return b->buffer->info.key_size + b->buffer->info.value_size;
}

unsigned char *vms_shm_dbg_buffer_data(vms_shm_dbg_buffer *b) {
    return b->data;
}

void vms_shm_dbg_buffer_inc_size(vms_shm_dbg_buffer *b, size_t size) {
    b->buffer->info.size += size;
}
size_t vms_shm_dbg_buffer_version(vms_shm_dbg_buffer *b) {
    return b->buffer->info.version;
}
void vms_shm_dbg_buffer_bump_version(vms_shm_dbg_buffer *b) {
    ++b->buffer->info.version;
}
