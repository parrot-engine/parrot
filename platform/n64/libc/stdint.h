#ifndef __SRC_PARROT_PLATFORM_N64_LIBC_STDINT_H_
#define __SRC_PARROT_PLATFORM_N64_LIBC_STDINT_H_

#include <limits.h>

#define INT8_MIN CHAR_MIN
#define INT8_MAX CHAR_MAX
#define INT16_MIN SHRT_MIN
#define INT16_MAX SHRT_MAX
#define INT32_MIN INT_MIN
#define INT32_MAX INT_MAX
#define INT64_MIN LLONG_MIN
#define INT64_MAX LLONG_MAX

#define UINT8_MAX UCHAR_MAX
#define UINT16_MAX USHRT_MAX
#define UINT32_MAX UINT_MAX
#define UINT64_MAX ULLONG_MAX

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef int8_t int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;
typedef int64_t int_least64_t;

typedef uint8_t uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef uint64_t uint_least64_t;

typedef int32_t int_fast8_t;
typedef int32_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef int64_t int_fast64_t;

typedef uint32_t uint_fast8_t;
typedef uint32_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;

typedef int32_t intptr_t;
typedef uint32_t uintptr_t;

#endif // __SRC_PARROT_PLATFORM_N64_LIBC_STDINT_H_
