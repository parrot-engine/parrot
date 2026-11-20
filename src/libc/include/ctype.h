#ifndef PARROT_PARROT_SRC_CORE_LIBC_INCLUDE_CTYPE_H_
#define PARROT_PARROT_SRC_CORE_LIBC_INCLUDE_CTYPE_H_

#include "parrot/core/util.h"

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

#endif // PARROT_PARROT_SRC_CORE_LIBC_INCLUDE_CTYPE_H_
