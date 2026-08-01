#ifndef __SRC_PARROT_PLATFORM_N64_STDLIB_STDINT_H_
#define __SRC_PARROT_PLATFORM_N64_STDLIB_STDINT_H_

#define INT8_MIN -128
#define INT16_MIN -32768
#define INT32_MIN -2147483648
#define INT64_MIN -9223372036854775808

#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define INT64_MAX 9223372036854775807

#define UINT8_MAX 255
#define UINT16_MAX 65535
#define UINT32_MAX 4294967295
#define UINT64_MAX 18446744073709551615

#define INTPTR_MIN INT32_MIN
#define INTPTR_MAX INT32_MAX

#define UINTPTR_MAX UINT32_MAX

#define SIZE_MAX UINT32_MAX

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef int8_t int_fast8_t;
typedef int16_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef int64_t int_fast64_t;

typedef uint32_t uintptr_t;
typedef int32_t intptr_t;

typedef uint8_t uint_fast8_t;
typedef uint16_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;

#endif // __SRC_PARROT_PLATFORM_N64_STDLIB_STDINT_H_