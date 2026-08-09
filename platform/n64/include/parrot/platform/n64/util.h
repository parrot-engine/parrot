#ifndef PARROT_PLATFORM_N64_INCLUDE_PARROT_PLATFORM_N64_UTIL_H_
#define PARROT_PLATFORM_N64_INCLUDE_PARROT_PLATFORM_N64_UTIL_H_

#include "parrot/core/api.h"
#include <stddef.h>

#define PARROT_N64_MSYNC() __asm__ volatile("sync")
#define PARROT_N64_MSYNC_DEPEND(...) __asm__ volatile("sync" ::__VA_ARGS__ : "memory")
#define PARROT_N64_MSYNC_FULL() __asm__ volatile("sync 1")
#define PARROT_N64_MSYNC_FULL_DEPEND(...) __asm__ volatile("sync 1" ::__VA_ARGS__ : "memory")

#define PARROT_N64_IO_WRITE(reg, value)                                                                                 \
    do {                                                                                                                \
        reg = value;                                                                                                    \
        PARROT_N64_MSYNC();                                                                                             \
    } while (0)

#define PARROT_N64_PHYSICAL_ADDRESS(addr) ((uintptr_t)((size_t)(addr) & 0x1FFFFFF))
#define PARROT_N64_UNCACHED_ADDRESS(addr) ((void *)(PARROT_N64_PHYSICAL_ADDRESS(addr) + 0xA0000000))
#define PARROT_N64_CACHED_ADDRESS(addr) ((void *)(PARROT_N64_PHYSICAL_ADDRESS(addr) + 0x80000000))

#define PARROT_N64_IS_UNCACHED(addr) (((uintptr_t)(addr) >> 29) == 0x5)
#define PARROT_N64_IS_CACHED(addr) (((uintptr_t)(addr) >> 29) == 0x4)

#define PARROT_N64_CACHE()

PARROT_API void Parrot_n64_invalidiate_instruction_cache(const void *addr, size_t size);
PARROT_API void Parrot_n64_invalidiate_data_cache(const void *addr, size_t size);

PARROT_API void Parrot_n64_writeback_invalidiate_data_cache(const void *addr, size_t size);

#endif // PARROT_PLATFORM_N64_INCLUDE_PARROT_PLATFORM_N64_UTIL_H_
