#include "platform/n64/stdlib/stdio.h"
#include "parrot/core/util.h"
#include "platform/n64/stdlib/errno.h"
#include "platform/n64/stdlib/stdarg.h"
#include "src/platform/n64/os/nanoprintf.h"
#include <stdint.h>
#include <string.h>

struct FILE {
    uint8_t pad;
};

int fprintf(FILE *restrict stream, const char *restrict format, ...) {
    va_list args;
    va_start(args, format);
    int result = vfprintf(stream, format, args);
    va_end(args);
    return result;
}

int printf(const char *restrict format, ...) {
    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
    va_end(args);
    return result;
}

int snprintf(char *restrict s, size_t n, const char *restrict format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(s, n, format, args);
    va_end(args);
    return result;
}

int sprintf(char *restrict s, const char *restrict format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsprintf(s, format, args);
    va_end(args);
    return result;
}

typedef enum {
    PrintfCallbackType_ISVIEWER = 0,
    PrintfCallbackType_STRING,
} PrintfCallbackType;

static void printf_callback(int c, void *ctx) {
    (void)ctx;

    void *isviewer_base_address = (void *)0xB3FF0000;

    char *isviewer_buffer = (char *)((uint8_t *)((uint8_t *)isviewer_base_address + 0x20));
    uint32_t *isviewer_write = ((uint32_t *)((uint8_t *)isviewer_base_address + 0x14));
    isviewer_buffer[0] = c;
    *isviewer_write = sizeof(char);
}

int vfprintf(FILE *restrict stream, const char *restrict format, va_list arg) {
    if (stream != stdout && stream != stderr) {
        errno = EBADF;
        return -1;
    }

    return npf_vpprintf(printf_callback, NULL, format, arg);
}

int vprintf(const char *restrict format, va_list arg) {
    return vfprintf(stdout, format, arg);
}

int vsnprintf(char *restrict s, size_t n, const char *restrict format, va_list arg) {
    PARROT_RET_COND_V(n == 0, 0);
    return npf_vsnprintf(s, n, format, arg);
}

int vsprintf(char *restrict s, const char *restrict format, va_list arg) {
    return npf_vsnprintf(s, SIZE_MAX, format, arg);
}

FILE *stdout = (FILE *)0xDEADBEEF;
FILE *stderr = (FILE *)0xDEADBEEF;