#ifndef VAMOS_UTILS_H_
#define VAMOS_UTILS_H_

#include <stddef.h> /* size_t */
#include <stdint.h>

int sleep_ns(uint64_t ns);
int sleep_ms(uint64_t ms);

/* On x86 and ARM the cache line has 64 bytes, change if needed. */
#define CACHELINE_SIZE 64
#define CACHELINE_ALIGNED _Alignas(CACHELINE_SIZE)
#define ADDR_IS_CACHE_ALIGNED(addr) \
    (((unsigned long long)(addr)) % CACHELINE_SIZE == 0)

/* Allocate memory and abort if the allocation fails */
void *xalloc(size_t size);
/* Allocate aligned memory and abort if the allocation fails */
void *xalloc_aligned(size_t size, size_t alignment);

char *xstrdup(const char *str);

#endif /* VAMOS_UTILS_H_ */
