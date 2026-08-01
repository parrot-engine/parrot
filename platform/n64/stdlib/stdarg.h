#ifndef __SRC_PARROT_PLATFORM_N64_STDLIB_STDARG_H_
#define __SRC_PARROT_PLATFORM_N64_STDLIB_STDARG_H_

typedef void *va_list;

#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_end(ap) __builtin_va_end(ap)
#define va_copy(dest, src) __builtin_va_copy(dest, src)

#endif // __SRC_PARROT_PLATFORM_N64_STDLIB_STDARG_H_