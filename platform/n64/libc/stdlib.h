#ifndef __SRC_PARROT_PLATFORM_N64_LIBC_STDLIB_H_
#define __SRC_PARROT_PLATFORM_N64_LIBC_STDLIB_H_

#include "parrot/core/api.h"
#include <stddef.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define RAND_MAX 2147483647

PARROT_API double atof(const char *nptr);
PARROT_API int atoi(const char *nptr);
PARROT_API long int atol(const char *nptr);
PARROT_API long long int atoll(const char *nptr);

PARROT_API double strtod(const char *restrict n, char **restrict endptr);
PARROT_API float strtof(const char *restrict n, char **restrict endptr);
PARROT_API long double strtold(const char *restrict n, char **restrict endptr);

PARROT_API long int strtol(const char *nptr, char **endptr, int base);
PARROT_API long long int strtoll(const char *nptr, char **endptr, int base);
PARROT_API unsigned long int strtoul(const char *restrict n, char **restrict endptr, int base);
PARROT_API unsigned long long int strtoull(const char *nptr, char **endptr, int base);

PARROT_API int rand(void);
PARROT_API void srand(unsigned int seed);

PARROT_API void *calloc(size_t nmemb, size_t size);
PARROT_API void free(void *ptr);
PARROT_API void *malloc(size_t size);
PARROT_API void *realloc(void *ptr, size_t size);

PARROT_API void abort(void);

PARROT_API int atexit(void (*func)(void));
PARROT_API void exit(int status);

PARROT_API char *getenv(const char *name);

PARROT_API int system(const char *string);

typedef int (*ParrotCompareFn)(const void *, const void *);

PARROT_API void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, ParrotCompareFn compare);
PARROT_API void qsort(void *base, size_t nmemb, size_t size, ParrotCompareFn compare);

PARROT_API int abs(int j);
PARROT_API long int labs(long int j);
PARROT_API long long int llabs(long long j);

#endif // __SRC_PARROT_PLATFORM_N64_LIBC_STDLIB_H_