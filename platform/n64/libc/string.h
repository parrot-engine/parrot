#ifndef __SRC_PARROT_PLATFORM_N64_LIBC_STRING_H_
#define __SRC_PARROT_PLATFORM_N64_LIBC_STRING_H_

#include "parrot/core/api.h"
#include <string.h>

PARROT_API void *memcpy(void *s1, const void *s2, size_t n);
PARROT_API void *memmove(void *s1, const void *s2, size_t n);

PARROT_API char *strcpy(char *s1, const char *s2);
PARROT_API char *strncpy(char *s1, const char *s2, size_t n);
PARROT_API char *strcat(char *s1, const char *s2);
PARROT_API char *strncat(char *s1, const char *s2, size_t n);

PARROT_API int memcmp(const void *s1, const void *s2, size_t n);
PARROT_API int strcmp(const char *s1, const char *s2);
PARROT_API int strncmp(const char *s1, const char *s2, size_t n);

PARROT_API void *memchr(const void *s, int c, size_t n);
PARROT_API char *strchr(const char *s, int c);
PARROT_API size_t strcspn(const char *s1, const char *s2);
PARROT_API char *strpbrk(const char *s1, const char *s2);
PARROT_API char *strrchr(const char *s, int c);
PARROT_API size_t strspn(const char *s1, const char *s2);
PARROT_API char *strstr(const char *s1, const char *s2);
PARROT_API char *strtok(char *s1, const char *s2);

PARROT_API void *memset(void *s, int c, size_t n);
PARROT_API char *strerror(int errnum);

PARROT_API size_t strlen(const char *s);

#endif // __SRC_PARROT_PLATFORM_N64_LIBC_STRING_H_