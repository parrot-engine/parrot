#ifndef __SRC_PARROT_PLATFORM_N64_LIBC_ASSERT_H_
#define __SRC_PARROT_PLATFORM_N64_LIBC_ASSERT_H_

#include "parrot/core/api.h"

PARROT_API void Parrot_os_assert(const char *file, int line, const char *func, const char *expr);

#define assert(expr) ((void)((!(expr)) ? Parrot_os_assert(__FILE__, __LINE__, __func__, #expr) : (void)0))

#endif // __SRC_PARROT_PLATFORM_N64_LIBC_ASSERT_H_