#ifndef __SRC_PARROT_PLATFORM_N64_LIBC_CTYPE_H_
#define __SRC_PARROT_PLATFORM_N64_LIBC_CTYPE_H_

#include "parrot/core/api.h"

PARROT_API int isalnum(int c);
PARROT_API int isalpha(int c);
PARROT_API int iscntrl(int c);
PARROT_API int isdigit(int c);
PARROT_API int isgraph(int c);
PARROT_API int islower(int c);
PARROT_API int isprint(int c);
PARROT_API int isspace(int c);
PARROT_API int isupper(int c);
PARROT_API int isxdigit(int c);

#endif // __SRC_PARROT_PLATFORM_N64_LIBC_CTYPE_H_