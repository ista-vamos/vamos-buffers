#include <assert.h>
#include <stdio.h>
#include <sys/mman.h>

#include "buffer-private.h"
#include "core/source.h"
#include "shm.h"

HIDE_SYMBOL
struct source_control *get_shared_control_buffer(const char *buff_key) {
    char key[SHM_NAME_MAXLEN];
    shamon_map_ctrl_key(buff_key, key);

    printf("Getting control buffer '%s'\n", key);
    int fd = shamon_shm_open(key, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        perror("shm_open");
        return NULL;
    }

    size_t size;
    if (pread(fd, &size, sizeof(size), 0) == -1) {
        perror("reading size of ctrl buffer");
        return NULL;
    }
    printf("   ... its size is %lu\n", size);

    void *mem = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
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
    return (struct source_control *)mem;
}

HIDE_SYMBOL
struct source_control *create_shared_control_buffer(
    const char *buff_key, mode_t mode, const struct source_control *control) {
    char key[SHM_NAME_MAXLEN];
    shamon_map_ctrl_key(buff_key, key);
    size_t size = control->size;

    printf("Initializing control buffer '%s' of size '%lu'\n", key, size);

    char tmpkey[SHM_NAME_MAXLEN] = "";
    if (shamon_get_tmp_key(key, tmpkey, SHM_NAME_MAXLEN) == -1) {
        fprintf(stderr, "Failed creating a tmpkey for '%s'\n", key);
        return NULL;
    }

    int fd = shamon_shm_open(tmpkey, O_RDWR | O_CREAT, mode);
    if (fd < 0) {
        perror("shm_open");
        return NULL;
    }

    /* The user does not want a control buffer, but we expect to have it,
     * event if it is empty. Make the size be at least such that it can
     * hold the size variable */
    if (size == 0) {
        size = sizeof(control->size);
    }
    assert(size >= sizeof(control->size));

    if ((ftruncate(fd, size)) == -1) {
        perror("ftruncate");
        return NULL;
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
        return NULL;
    }

    memcpy(mem, control, size);

    if (shamon_shm_rename(tmpkey, key) < 0) {
        perror("renaming SHM file");

        if (close(fd) == -1) {
            perror("closing fd after mmap failure");
        }
        if (shamon_shm_unlink(tmpkey) != 0) {
            perror("shm_unlink after mmap failure");
        }
        if (munmap(mem, size) != 0) {
            perror("munmap of control buffer");
        }
        return NULL;
    }

    return (struct source_control *)mem;
}

HIDE_SYMBOL
void release_shared_control_buffer(struct source_control *buffer) {
    if (munmap(buffer, buffer->size) != 0) {
        perror("munmap failure");
    }

    /* TODO: we leak fd */
}

HIDE_SYMBOL
void destroy_shared_control_buffer(const char *buffkey,
                                   struct source_control *buffer) {
    release_shared_control_buffer(buffer);

    char key[SHM_NAME_MAXLEN];
    shamon_map_ctrl_key(buffkey, key);
    if (shamon_shm_unlink(key) != 0) {
        perror("destroy_shared_control_buffer: shm_unlink failure");
    }
}
