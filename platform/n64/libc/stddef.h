#ifndef __SRC_PARROT_PLATFORM_N64_LIBC_STDDEF_H_
#define __SRC_PARROT_PLATFORM_N64_LIBC_STDDEF_H_

#include <stdint.h>

typedef uint64_t size_t;
typedef int64_t ssize_t;

typedef int64_t ptrdiff_t;

#define NULL ((void *)0)

#endif // __SRC_PARROT_PLATFORM_N64_LIBC_STDDEF_H_