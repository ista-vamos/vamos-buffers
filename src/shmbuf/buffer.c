#include "vamos-buffers/shmbuf/buffer.h"

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

#include <immintrin.h> /* _mm_mfence */

#include "buffer-private.h"
#include "shm.h"
#include "vamos-buffers/core/list.h"
#include "vamos-buffers/core/source.h"
#include "vamos-buffers/core/spsc_ringbuf.h"
#include "vamos-buffers/core/utils.h"
#include "vamos-buffers/core/vector-macro.h"

bool vms_shm_buffer_is_destroyed(vms_shm_buffer *buff) {
    return buff->shmbuffer->info.destroyed;
}

bool vms_shm_buffer_is_ready(vms_shm_buffer *buff) {
    return !vms_shm_buffer_is_destroyed(buff);
}

bool vms_shm_buffer_reader_is_ready(vms_shm_buffer *buff) {
    return (buff->shmbuffer->info.flags & READER_IS_READY);
}

size_t vms_shm_buffer_capacity(vms_shm_buffer *buff) {
    return buff->shmbuffer->info.capacity;
}

size_t vms_shm_buffer_size(vms_shm_buffer *buff) {
    return vms_spsc_ringbuf_size(_ringbuf(buff));
}

size_t vms_shm_buffer_elem_size(vms_shm_buffer *buff) {
    return buff->shmbuffer->info.elem_size;
}

const char *vms_shm_buffer_key(vms_shm_buffer *buffer) { return buffer->key; }

int vms_shm_buffer_compute_key_path(vms_shm_buffer *buff, char keypath[],
                                    size_t keypathsize) {
    if (SHM_NAME_MAXLEN <= keypathsize)
        return -1;

    if (vms_shm_mapname(buff->key, keypath) == NULL) {
        return -2;
    }

    return 0;
}

int vms_shm_buffer_compute_ctrl_key_path(vms_shm_buffer *buff, char keypath[],
                                         size_t keypathsize) {
    if (SHM_NAME_MAXLEN <= keypathsize)
        return -1;

    char ctrlkey[SHM_NAME_MAXLEN];
    if (vms_shm_map_ctrl_key(buff->key, ctrlkey) == NULL) {
        return -3;
    }

    if (vms_shm_mapname(ctrlkey, keypath) == NULL) {
        return -2;
    }

    return 0;
}

/* Pointer to the beginning of the allocated memory. */
#define BUFF_START(b) ((unsigned char *)b->data)
/* Pointer to the first byte after the allocated memory. We allocate 1 more byte
   than is the desired capacity. */
#define BUFF_END(b) \
    ((unsigned char *)b->data + (b->info.elem_size * (b->info.capacity + 1)))

HIDE_SYMBOL
vms_shm_buffer *_vms_shm_buffer_initialize(const char *key, mode_t mode,
                                           size_t elem_size, size_t capacity,
                                           struct vms_source_control *control) {
    assert(elem_size > 0 && "Element size is 0");
    assert(capacity > 0 && "Capacity is 0");
    /* the ringbuffer has one unusable dummy element, so increase the capacity
     * by one */
    const size_t memsize = compute_vms_size(elem_size, capacity + 1);

#ifndef NDEBUG
    fprintf(stderr,
            "[vamos] init buffer '%s': elem-size '%lu', "
            "capacity '%lu' (%luB => %lu pages)\n",
            key, elem_size, capacity, memsize, memsize / PAGE_SIZE);
#endif

    /* We first create a temporary key and open the SHM file with that key
     * instead of the required key. Then we can initialize the shared memory
     * and only then we rename the file to use the original key. This way
     * we avoid a race when the other side opens the SHM file and reads from
     * it before it is fully initialized */
    char tmpkey[SHM_NAME_MAXLEN] = "";
    if (vms_shm_get_tmp_key(key, tmpkey, SHM_NAME_MAXLEN) == -1) {
        fprintf(stderr, "Failed creating a tmpkey for '%s'\n", key);
        return NULL;
    }

    int fd = vms_shm_open(tmpkey, O_RDWR | O_CREAT | O_TRUNC, mode);
    if (fd < 0) {
        perror("vms_shm_open");
        return NULL;
    }

    if ((ftruncate(fd, memsize)) == -1) {
        perror("ftruncate");
        return NULL;
    }

    void *shmem = mmap(0, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shmem == MAP_FAILED) {
        perror("mmap failure");
        if (close(fd) == -1) {
            perror("closing fd after mmap failure");
        }
        if (vms_shm_unlink(tmpkey) != 0) {
            perror("vms_unlink after mmap failure");
        }
        return NULL;
    }

    vms_shm_buffer *buff = xalloc(sizeof(vms_shm_buffer));
    buff->shmbuffer = (struct shmbuffer *)shmem;
    assert(ADDR_IS_CACHE_ALIGNED(buff->shmbuffer->data));
    assert(ADDR_IS_CACHE_ALIGNED(&buff->shmbuffer->info.ringbuf));

    memset(buff->shmbuffer, 0, sizeof(vms_shm_buffer_info));

    buff->shmbuffer->info.allocated_size = memsize;
    buff->shmbuffer->info.capacity = capacity;
    /* ringbuf has one dummy element and we allocated the space for it */
    vms_spsc_ringbuf_init(_ringbuf(buff), capacity + 1);
    buff->shmbuffer->info.elem_size = elem_size;
    buff->shmbuffer->info.last_processed_id = 0;
    buff->shmbuffer->info.dropped_ranges_next = 0;
    buff->shmbuffer->info.dropped_ranges_lock = false;
    buff->shmbuffer->info.subbuffers_no = 0;

#if 0
    fprintf(stderr, "  .. buffer allocated size = %lu, capacity = %lu\n",
            buff->shmbuffer->info.allocated_size,
            buff->shmbuffer->info.capacity);
    fprintf(stderr, "  .. buffer memory range:  %p - %p\n",
            (void *)BUFF_START(buff->shmbuffer),
            (void *)BUFF_END(buff->shmbuffer));
#endif

#ifndef NDEBUG
    assert((size_t)(BUFF_END(buff->shmbuffer) - BUFF_START(buff->shmbuffer)) ==
           (capacity + 1) * elem_size);
    /* In debugging mode, set the allocated memory to test if it is really
     * accessible and that our structures are not (incorrectly) overlapping with
     * the memory */
    memset(BUFF_START(buff->shmbuffer), 0xff, capacity * elem_size);
#endif

    buff->key = strdup(key);
    VEC_INIT(buff->aux_buffers);
    vms_list_init(&buff->aux_buffers_age);
    buff->aux_buf_idx = 0;
    buff->cur_aux_buff = NULL;
    buff->fd = fd;
    buff->control = control;
    buff->mode = mode;
    buff->last_subbufer_no = 0;

    if (vms_shm_rename(tmpkey, key) < 0) {
        perror("renaming SHM file");

        if (close(fd) == -1) {
            perror("closing fd after mmap failure");
        }
        if (vms_shm_unlink(tmpkey) != 0) {
            perror("vms_unlink after mmap failure");
        }
        free(buff);
        return NULL;
    }

    return buff;
}

vms_shm_buffer *vms_shm_buffer_create(
    const char *key, size_t capacity,
    const struct vms_source_control *control) {
    struct vms_source_control *ctrl =
        create_shared_control_buffer(key, S_IRWXU, control);
    if (!ctrl) {
        fprintf(stderr, "Failed creating control buffer\n");
        return NULL;
    }

    size_t elem_size = vms_source_control_max_event_size(ctrl);
    return _vms_shm_buffer_initialize(key, S_IRWXU, elem_size, capacity, ctrl);
}

vms_shm_buffer *vms_shm_buffer_create_adv(
    const char *key, mode_t mode, size_t elem_size, size_t capacity,
    const struct vms_source_control *control) {
    struct vms_source_control *ctrl =
        create_shared_control_buffer(key, mode, control);
    if (!ctrl) {
        fprintf(stderr, "Failed creating control buffer\n");
        return NULL;
    }

    if (elem_size == 0) {
        elem_size = vms_source_control_max_event_size(ctrl);
    }

    if (mode == 0) {
        mode = S_IRWXU;
    }

    return _vms_shm_buffer_initialize(key, mode, elem_size, capacity, ctrl);
}

vms_shm_buffer *vms_shm_buffer_try_connect(const char *key, size_t retry) {
    // fprintf(stderr, "getting shared buffer '%s'\n", key);

    int fd = -1;
    ++retry;
    do {
        fd = vms_shm_open(key, O_RDWR, S_IRWXU);
        if (fd >= 0) {
            break;
        }
        sleep_ms(300);
    } while (--retry > 0);

    if (fd == -1) {
        perror("vms_shm_open");
        fprintf(stderr, "Failed getting shared buffer '%s'\n", key);
        return NULL;
    }

    vms_shm_buffer_info info;
    if (pread(fd, &info, sizeof(info), 0) == -1) {
        perror("reading info of shared buffer");
        close(fd);
        return NULL;
    }

    // fprintf(stderr, "   ... its size is %lu\n", info.allocated_size);
    if (info.allocated_size == 0) {
        fprintf(stderr, "Invalid allocated size of SHM buffer: %lu\n",
                info.allocated_size);
        close(fd);
        return NULL;
    }

    void *shmmem =
        mmap(0, info.allocated_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shmmem == MAP_FAILED) {
        perror("mmap failure");
        goto before_mmap_clean;
    }

    vms_shm_buffer *buff = malloc(sizeof(*buff));
    if (!buff) {
        fprintf(stderr, "%s:%d: memory allocation failed\n", __func__,
                __LINE__);
        goto mmap_clean;
    }

    buff->key = strdup(key);
    if (!buff->key) {
        fprintf(stderr, "%s:%d: memory allocation failed\n", __func__,
                __LINE__);
        goto buff_clean_key;
    }

    VEC_INIT(buff->aux_buffers);

    buff->control = get_shared_control_buffer(key);
    if (!buff->control) {
        fprintf(stderr, "%s:%d: failed getting control buffer\n", __func__,
                __LINE__);
        goto buff_clean_all;
    }

    buff->shmbuffer = (struct shmbuffer *)shmmem;
    buff->aux_buf_idx = 0;
    buff->cur_aux_buff = NULL;
    buff->fd = fd;
    buff->mode = 0;

    return buff;

buff_clean_all:
    VEC_DESTROY(buff->aux_buffers);
    free(buff->key);
buff_clean_key:
    free(buff);
mmap_clean:
    munmap(shmmem, info.allocated_size);
before_mmap_clean:
    if (close(fd) == -1) {
        perror("closing fd after mmap failure");
    }
    if (vms_shm_unlink(key) != 0) {
        perror("vms_unlink after mmap failure");
    }
    return NULL;
}

vms_shm_buffer *vms_shm_buffer_connect(const char *key) {
    return vms_shm_buffer_try_connect(key, 10);
}

struct vms_event_record *vms_shm_buffer_get_avail_events(vms_shm_buffer *buff,
                                                         size_t *evs_num) {
    assert(buff);
    assert(evs_num);
    assert(buff->control);

    /* make sure that concurrent writes to the source control buffer are visible now */
    _mm_mfence();
    *evs_num = vms_source_control_get_records_num(buff->control);
    return buff->control->events;
}

void vms_shm_buffer_set_flags(vms_shm_buffer *buff, uint64_t flags) {
    buff->shmbuffer->info.flags |= flags;
}

void vms_shm_buffer_unset_flags(vms_shm_buffer *buff, uint64_t flags) {
    buff->shmbuffer->info.flags &= (~flags);
}

void vms_shm_buffer_set_reader_is_ready(vms_shm_buffer *buff) {
    if (!buff->shmbuffer->info.destroyed)
        buff->shmbuffer->info.flags |= READER_IS_READY;
}

/* set the ID of the last processed event */
void vms_shm_buffer_set_last_processed_id(vms_shm_buffer *buff,
                                          vms_eventid id) {
    assert(buff->shmbuffer->info.last_processed_id <= id &&
           "The IDs are not monotonic");
    buff->shmbuffer->info.last_processed_id = id;
}

/* for readers */
void vms_shm_buffer_release(vms_shm_buffer *buff) {
    if (munmap(buff->shmbuffer, buff->shmbuffer->info.allocated_size) != 0) {
        perror("release_shared_buffer: munmap failure");
    }
    if (close(buff->fd) == -1) {
        perror("release_shared_buffer: failed closing mmap fd");
    }

    size_t vecsize = VEC_SIZE(buff->aux_buffers);
    for (size_t i = 0; i < vecsize; ++i) {
        struct aux_buffer *ab = buff->aux_buffers[i];
        aux_buffer_release(ab);
    }
    VEC_DESTROY(buff->aux_buffers);

    release_shared_control_buffer(buff->control);

    free(buff->key);
    free(buff);
}

/* for writers */
void vms_shm_buffer_destroy(vms_shm_buffer *buff) {
    buff->shmbuffer->info.destroyed = 1;

    size_t vecsize = VEC_SIZE(buff->aux_buffers);
    for (size_t i = 0; i < vecsize; ++i) {
        struct aux_buffer *ab = buff->aux_buffers[i];
        // we must first wait until the monitor finishes
        // aux_buffer_destroy(ab);
        aux_buffer_release(ab);
    }
    VEC_DESTROY(buff->aux_buffers);
#ifndef NDEBUG
    fprintf(stderr, "[vamos] '%s' used %lu aux buffers\n", buff->key, vecsize);
#endif

    if (munmap(buff->shmbuffer, buff->shmbuffer->info.allocated_size) != 0) {
        perror("vms_shm_buffer_destroy: munmap failure");
    }
    if (close(buff->fd) == -1) {
        perror("vms_shm_buffer_destroy: failed closing mmap fd");
    }
    if (vms_shm_unlink(buff->key) != 0) {
        perror("vms_shm_buffer_destroy: vms_unlink failure");
    }

    destroy_shared_control_buffer(buff->key, buff->control);

    free(buff->key);
    free(buff);
}

void *vms_shm_buffer_read_pointer(vms_shm_buffer *buff, size_t *size) {
    vms_shm_buffer_info *info = &buff->shmbuffer->info;
    size_t tail = vms_spsc_ringbuf_read_off_nowrap(&info->ringbuf, size);
    if (*size == 0)
        return NULL;
    /* TODO: get rid of the multiplication,
     * incrementally shift a pointer instead */
    return buff->shmbuffer->data + tail * info->elem_size;
}

bool vms_shm_buffer_drop_k(vms_shm_buffer *buff, size_t k) {
    return vms_spsc_ringbuf_consume_upto(_ringbuf(buff), k) == k;
}

size_t vms_shm_buffer_consume(vms_shm_buffer *buff, size_t k) {
    return vms_spsc_ringbuf_consume_upto(_ringbuf(buff), k);
}

/* buffer_push broken down into several operations:
 *
 *  p = vms_shm_buffer_start_push(...)
 *  p = vms_shm_buffer_partial_push(..., p, ...)
 *  ...
 *  p = vms_shm_buffer_partial_push(..., p, ...)
 *  vms_shm_buffer_partial_push(..., p, ...)
 *  vms_shm_buffer_finish_push(...)
 *
 * All the partial push may push only a single element in sum,
 * i.e., what can be done with buffer_push. Partial pushes
 * cannot be mixed nor combined with normal push operations.
 * These are really just one buffer_push broken down into
 * multiple steps.
 */

void *vms_shm_buffer_start_push(vms_shm_buffer *buff) {
    vms_shm_buffer_info *info = &buff->shmbuffer->info;
    assert(!info->destroyed && "Writing to a destroyed buffer");

    size_t n;
    size_t off = vms_spsc_ringbuf_write_off_nowrap(_ringbuf(buff), &n);
    if (n == 0) {
        /* ++info->dropped; */
        return NULL;
    }

    /* all ok, return the pointer to the data */
    /* FIXME: do not use multiplication, maintain the pointer to the head of
     * data */
    void *mem = buff->shmbuffer->data + off * info->elem_size;
    assert((void *)BUFF_START(buff->shmbuffer) <= mem);
    assert(mem < (void *)BUFF_END(buff->shmbuffer));
    return mem;
}

void *vms_shm_buffer_partial_push(vms_shm_buffer *buff, void *prev_push,
                                  const void *elem, size_t size) {
    assert(!vms_shm_buffer_is_destroyed(buff) &&
           "Writing to a destroyed buffer");
    assert(BUFF_START(buff->shmbuffer) <= (unsigned char *)prev_push);
    assert((unsigned char *)prev_push < BUFF_END(buff->shmbuffer));
    assert((unsigned char *)prev_push <= BUFF_END(buff->shmbuffer) - size);
    (void)buff;

    memcpy(prev_push, elem, size);
    return (unsigned char *)prev_push + size;
}

static size_t _buffer_push_strn(vms_shm_buffer *buff, const void *data,
                                size_t size);
static uint64_t buffer_push_str(vms_shm_buffer *buff, uint64_t evid,
                                const char *str);
static uint64_t buffer_push_strn(vms_shm_buffer *buff, uint64_t evid,
                                 const char *str, size_t len);

void *vms_shm_buffer_partial_push_str(vms_shm_buffer *buff, void *prev_push,
                                      uint64_t evid, const char *str) {
    assert(!buff->shmbuffer->info.destroyed && "Writing to a destroyed buffer");
    assert(BUFF_START(buff->shmbuffer) <= (unsigned char *)prev_push);
    assert((unsigned char *)prev_push < BUFF_END(buff->shmbuffer));

    *((uint64_t *)prev_push) = buffer_push_str(buff, evid, str);
    /*printf("Pushed str: %lu\n", *((uint64_t *)prev_push));*/
    return (unsigned char *)prev_push + sizeof(uint64_t);
}

void *vms_shm_buffer_partial_push_str_n(vms_shm_buffer *buff, void *prev_push,
                                        uint64_t evid, const char *str,
                                        size_t len) {
    assert(!buff->shmbuffer->info.destroyed && "Writing to a destroyed buffer");
    assert(BUFF_START(buff->shmbuffer) <= (unsigned char *)prev_push);
    assert((unsigned char *)prev_push < BUFF_END(buff->shmbuffer));

    *((uint64_t *)prev_push) = buffer_push_strn(buff, evid, str, len);
    /*printf("Pushed str: %lu\n", *((uint64_t *)prev_push));*/
    return (unsigned char *)prev_push + sizeof(uint64_t);
}

void vms_shm_buffer_finish_push(vms_shm_buffer *buff) {
    assert(!buff->shmbuffer->info.destroyed && "Writing to a destroyed buffer");
    vms_spsc_ringbuf_write_finish(_ringbuf(buff), 1);
}

bool vms_shm_buffer_push(vms_shm_buffer *buff, const void *elem, size_t size) {
    assert(!buff->shmbuffer->info.destroyed && "Writing to a destroyed buffer");
    assert(buff->shmbuffer->info.elem_size >= size &&
           "Size does not fit the slot");

    void *dst = vms_shm_buffer_start_push(buff);
    if (dst == NULL)
        return false;

    memcpy(dst, elem, size);
    vms_shm_buffer_finish_push(buff);

    return true;
}

bool vms_shm_buffer_pop(vms_shm_buffer *buff, void *dst) {
    assert(!buff->shmbuffer->info.destroyed &&
           "Reading from a destroyed buffer");

    size_t size;
    void *pos = vms_shm_buffer_read_pointer(buff, &size);
    if (size > 0) {
        memcpy(dst, pos, buff->shmbuffer->info.elem_size);
        vms_spsc_ringbuf_consume(_ringbuf(buff), 1);
        return true;
    }

    return false;
}

size_t _buffer_push_strn(vms_shm_buffer *buff, const void *data, size_t size) {
    struct aux_buffer *ab = writer_get_aux_buffer(buff, size);
    assert(ab);
    assert(ab == buff->cur_aux_buff);
    assert(vms_list_last(&buff->aux_buffers_age)->data == buff->cur_aux_buff);

    size_t off = ab->head;
    assert(off < (1LU << 32));

    memcpy(ab->data + off, data, size);
    ab->head += size;
    return off;
}

void *vms_shm_buffer_read_str(vms_shm_buffer *buff, uint64_t elem) {
    size_t idx = elem >> 32;
    struct aux_buffer *ab = reader_get_aux_buffer(buff, idx);
    size_t off = elem & 0xffffffff;
    return ab->data + off;
}

uint64_t buffer_push_str(vms_shm_buffer *buff, uint64_t evid, const char *str) {
    size_t len = strlen(str) + 1;
    size_t off = buffer_push_strn(buff, evid, str, len);
    assert(buff->cur_aux_buff);
    return (off | (buff->cur_aux_buff->idx << 32));
}

uint64_t buffer_push_strn(vms_shm_buffer *buff, uint64_t evid, const char *str,
                          size_t len) {
    size_t off = _buffer_push_strn(buff, str, len);
    struct aux_buffer *ab = buff->cur_aux_buff;
    assert(ab);
    if (ab->first_event_id == 0)
        ab->first_event_id = evid;
    ab->last_event_id = evid;
    return (off | (buff->cur_aux_buff->idx << 32));
}

void vms_shm_buffer_notify_dropped(vms_shm_buffer *buff, uint64_t begin_id,
                                   uint64_t end_id) {
    vms_shm_buffer_info *info = &buff->shmbuffer->info;
    size_t idx = info->dropped_ranges_next;
    struct dropped_range *r = &info->dropped_ranges[idx];
    if (r->begin == begin_id || r->end == r->begin - 1) {
        drop_ranges_lock(buff);
        r->end = end_id;
        drop_ranges_unlock(buff);
        return;
    }

    if (idx + 1 == DROPPED_RANGES_NUM) {
        r = &info->dropped_ranges[0];
        info->dropped_ranges_next = 0;
    } else {
        ++r;
        ++info->dropped_ranges_next;
    }

    drop_ranges_lock(buff);
    r->begin = begin_id;
    r->end = end_id;
    drop_ranges_unlock(buff);
}

int vms_shm_buffer_register_event(vms_shm_buffer *b, const char *name,
                                  uint64_t kind) {
    assert(kind > vms_event_get_last_special_kind() && "Invalid event kind, it overlaps special kinds");
    struct vms_event_record *rec = vms_source_control_get_event(b->control, name);
    if (rec == NULL) {
        return -1;
    }

    rec->kind = kind;
    return 0;
}

int vms_shm_buffer_register_events(vms_shm_buffer *b, size_t ev_nums, ...) {
    va_list ap;
    va_start(ap, ev_nums);

    for (size_t i = 0; i < ev_nums; ++i) {
        const char *name = va_arg(ap, const char *);
        vms_kind kind = va_arg(ap, vms_kind);
        assert(kind > vms_event_get_last_special_kind() && "Invalid event kind, it overlaps special kinds");
        if (vms_shm_buffer_register_event(b, name, kind) < 0) {
            va_end(ap);
            return -1;
        }
    }

    va_end(ap);

    return 0;
}

int vms_shm_buffer_register_all_events(vms_shm_buffer *b) {
    struct vms_event_record *recs = b->control->events;
    const size_t ev_nums = vms_source_control_get_records_num(b->control);

    for (size_t i = 0; i < ev_nums; ++i) {
        recs[i].kind = 1 + i + vms_event_get_last_special_kind();
    }

    return 0;
}
