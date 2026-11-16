#include "nanoprintf.h"
#include "parrot/core/libc/yield.h"
#include "parrot/core/os.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int remove(const char *filename) {
    (void)filename;
    Parrot_libc_yield();
    return -1;
}

int rename(const char *old, const char *new) {
    (void)old;
    (void)new;
    Parrot_libc_yield();
    return -1;
}

FILE *tmpfile(void) {
    Parrot_libc_yield();
    return NULL;
}

char *tmpnam(char *s) {
    (void)s;
    Parrot_libc_yield();
    return NULL;
}

int fclose(FILE *stream) {
    (void)stream;
    Parrot_libc_yield();
    return EOF;
}

int fflush(FILE *stream) {
    (void)stream;
    Parrot_libc_yield();
    return EOF;
}

FILE *fopen(const char *filename, const char *mode) {
    (void)filename;
    (void)mode;
    Parrot_libc_yield();
    return NULL;
}

FILE *freopen(const char *filename, const char *mode, FILE *restrict stream) {
    (void)filename;
    (void)mode;
    (void)stream;
    Parrot_libc_yield();
    return NULL;
}

void setbuf(FILE *stream, char *buf) {
    (void)stream;
    (void)buf;
    Parrot_libc_yield();
}

int setvbuf(FILE *stream, char *buf, int mode, size_t size) {
    (void)stream;
    (void)buf;
    (void)mode;
    (void)size;
    Parrot_libc_yield();
    return -1;
}

int fprintf(FILE *restrict stream, const char *restrict format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(stream, format, args);
    va_end(args);
    return ret;
}

int fscanf(FILE *stream, const char *format, ...) {
    (void)stream;
    (void)format;
    Parrot_libc_yield();
    return EOF;
}

int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vprintf(format, args);
    va_end(args);
    return ret;
}

int scanf(const char *format, ...) {
    (void)format;
    Parrot_libc_yield();
    return EOF;
}

int snprintf(char *s, size_t n, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsnprintf(s, n, format, args);
    va_end(args);
    return ret;
}

int sprintf(char *s, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsprintf(s, format, args);
    va_end(args);
    return ret;
}

int sscanf(const char *s, const char *format, ...) {
    (void)s;
    (void)format;
    return EOF;
}

void vfprintf_putc(int c, void *ctx) {
    FILE *stream = (FILE *)ctx;
    fputc(c, stream);
}

int vfprintf(FILE *stream, const char *format, va_list arg) {
    return npf_vpprintf(vfprintf_putc, stream, format, arg);
}

int vfscanf(FILE *stream, const char *format, va_list arg) {
    (void)stream;
    (void)format;
    (void)arg;
    Parrot_libc_yield();
    return EOF;
}

int vprintf(const char *format, va_list arg) {
    return vfprintf(stdout, format, arg);
}

int vscanf(const char *format, va_list arg) {
    (void)format;
    (void)arg;
    return EOF;
}

int vsnprintf(char *s, size_t n, const char *format, va_list arg) {
    return npf_vsnprintf(s, n, format, arg);
}

void vsprintf_putc(int c, void *ctx) {
    char **ptr = (char **)ctx;
    *(*ptr)++ = c;
}

int vsprintf(char *s, const char *format, va_list arg) {
    char *ptr = s;
    int ret = npf_vpprintf(vsprintf_putc, &ptr, format, arg);
    Parrot_libc_yield();
    return ret;
}

int vsscanf(const char *s, const char *format, va_list arg) {
    (void)s;
    (void)format;
    (void)arg;
    Parrot_libc_yield();
    return EOF;
}

int fgetc(FILE *stream) {
    (void)stream;
    Parrot_libc_yield();
    return EOF;
}

char *fgets(char *restrict s, int n, FILE *restrict stream) {
    (void)s;
    (void)n;
    (void)stream;
    Parrot_libc_yield();
    return NULL;
}

int fputc(int c, FILE *stream) {
    (void)c;
    (void)stream;

    Parrot_libc_yield();
    return 0;
}

int fputs(const char *s, FILE *stream) {
    return fwrite(s, sizeof(char), strlen(s), stream);
}

int getc(FILE *stream) {
    (void)stream;
    Parrot_libc_yield();
    return EOF;
}

int getchar(void) {
    Parrot_libc_yield();
    return EOF;
}

char *gets(char *s) {
    (void)s;
    Parrot_libc_yield();
    return NULL;
}

int putc(int c, FILE *stream) {
    return fputc(c, stream);
}

int putchar(int c) {
    return fputc(c, stdout);
}

int puts(const char *s) {
    return fputs(s, stdout);
}

int ungetc(int c, FILE *stream) {
    (void)c;
    (void)stream;
    Parrot_libc_yield();
    return EOF;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    (void)ptr;
    (void)size;
    (void)nmemb;
    (void)stream;
    Parrot_libc_yield();
    return 0;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    (void)ptr;
    (void)size;
    (void)nmemb;
    (void)stream;

    Parrot_libc_yield();
    return 0;
}

int fgetpos(FILE *stream, fpos_t *pos) {
    (void)stream;
    (void)pos;
    Parrot_libc_yield();
    return -1;
}

int fseek(FILE *stream, long offset, int whence) {
    (void)stream;
    (void)offset;
    (void)whence;
    Parrot_libc_yield();
    return -1;
}

int fsetpos(FILE *stream, const fpos_t *pos) {
    (void)stream;
    (void)pos;
    Parrot_libc_yield();
    return -1;
}

long int ftell(FILE *stream) {
    (void)stream;
    Parrot_libc_yield();
    return -1L;
}

void rewind(FILE *stream) {
    (void)stream;
    Parrot_libc_yield();
}

void clearerr(FILE *stream) {
    (void)stream;
    Parrot_libc_yield();
}

int feof(FILE *stream) {
    (void)stream;
    Parrot_libc_yield();
    return 1;
}

int ferror(FILE *stream) {
    (void)stream;
    Parrot_libc_yield();
    return 1;
}

void perror(const char *s) {
    (void)s;
    Parrot_libc_yield();
}

#undef stdin
#undef stdout
#undef stderr

FILE *stdin = NULL;
FILE *stdout = (FILE *)0x1000;
FILE *stderr = (FILE *)0x2000;
