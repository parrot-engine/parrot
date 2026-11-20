#ifndef PARROT_PARROT_SRC_CORE_LIBC_INCLUDE_ASSERT_H_
#define PARROT_PARROT_SRC_CORE_LIBC_INCLUDE_ASSERT_H_

#include "parrot/core/util.h"

PARROT_API void Parrot_n64_os_assert(const char *file, int line, const char *func, const char *expr);

#define assert(expr) ((void)((!(expr)) ? Parrot_n64_os_assert(__FILE__, __LINE__, __func__, #expr) : (void)0))

#endif // PARROT_PARROT_SRC_CORE_LIBC_INCLUDE_ASSERT__H_
