#ifndef PARROT_PARROT_SRC_CORE_LIBC_INCLUDE_STDARG_H_
#define PARROT_PARROT_SRC_CORE_LIBC_INCLUDE_STDARG_H_

typedef char *va_list;
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_copy(dest, src) __builtin_va_copy(dest, src)
#define va_end(ap) __builtin_va_end(ap)
#define va_start(ap, param) __builtin_va_start(ap, param)

#endif // PARROT_PARROT_SRC_CORE_LIBC_INCLUDE_STDARG_H_
