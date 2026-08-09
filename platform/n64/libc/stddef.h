#ifndef __SRC_PARROT_PLATFORM_N64_LIBC_STDDEF_H_
#define __SRC_PARROT_PLATFORM_N64_LIBC_STDDEF_H_

#include <stdint.h>

typedef uint32_t size_t;
typedef int32_t ssize_t;

typedef ssize_t ptrdiff_t;

#define NULL ((void *)0)

#define offsetof(type, field) ((size_t)(uintptr_t)&((type *)0)->field)

#endif // __SRC_PARROT_PLATFORM_N64_LIBC_STDDEF_H_
