#ifndef PARROT_PARROT_PLATFORM_N64_INCLUDE_PARROT_PLATFORM_N64_OS_H_
#define PARROT_PARROT_PLATFORM_N64_INCLUDE_PARROT_PLATFORM_N64_OS_H_

#include "parrot/core/util.h"
#include <stddef.h>

typedef uint32_t ParrotOSPageTableFlags;

#define ParrotOSPageTableFlags_VALID ((ParrotOSPageTableFlags)(1 << 0))
#define ParrotOSPageTableFlags_UNCACHED ((ParrotOSPageTableFlags)(1 << 1))
#define ParrotOSPageTableFlags_WRITE ((ParrotOSPageTableFlags)(1 << 2))
#define ParrotOSPageTableFlags_EXEC ((ParrotOSPageTableFlags)(1 << 3))

PARROT_API void *Parrot_n64_os_alloc_pages(size_t count, ParrotOSPageTableFlags flags);
/// Returns non virtual pointer
PARROT_API void *Parrot_n64_os_alloc_continous_pages(size_t count);
PARROT_API void Parrot_n64_os_free_page(void *addr);
/// NULL = unmapped
PARROT_API void *Parrot_n64_os_get_physical_address(void *addr);
/// NULL = unmapped
PARROT_API void *Parrot_n64_os_get_physical_uncached_address(void *addr);
/// NULL = unmapped
PARROT_API void *Parrot_n64_os_get_physical_address_raw(void *addr);
/// NULL = unmapped
PARROT_API void *Parrot_n64_os_get_physical_uncached_address_raw(void *addr);

#endif // PARROT_PARROT_PLATFORM_N64_INCLUDE_PARROT_PLATFORM_N64_OS_H_
