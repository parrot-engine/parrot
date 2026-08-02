#ifndef __SRC_PARROT_PLATFORM_N64_LIBC_STDIO_H_
#define __SRC_PARROT_PLATFORM_N64_LIBC_STDIO_H_

#include "parrot/core/api.h"
#include <stdarg.h>
#include <stddef.h>

typedef struct FILE FILE;

typedef size_t fpos_t;

#define EOF 1024
#define FOPEN_MAX 32
#define FILENAME_MAX 32

#define TMP_MAX 0

extern char L_tmpnam[TMP_MAX + 1];

extern FILE *stderr;
extern FILE *stdin;
extern FILE *stdout;

PARROT_API int remove(const char *filename);
PARROT_API int rename(const char *old, const char *new);

PARROT_API FILE *tmpfile(void);
PARROT_API char *tmpnam(char *s);

PARROT_API int fclose(FILE *stream);
PARROT_API int fflush(FILE *stream);
PARROT_API FILE *fopen(const char *filename, const char *mode);
PARROT_API FILE *freopen(const char *filename, const char *mode, FILE *restrict stream);
PARROT_API void setbuf(FILE *stream, char *buf);
PARROT_API int setvbuf(FILE *stream, char *buf, int mode, size_t size);

PARROT_API int fprintf(FILE *restrict stream, const char *restrict format, ...);
PARROT_API int fscanf(FILE *stream, const char *format, ...);
PARROT_API int printf(const char *format, ...);
PARROT_API int scanf(const char *format, ...);
PARROT_API int snprintf(char *s, size_t n, const char *format, ...);
PARROT_API int sprintf(char *s, const char *format, ...);
PARROT_API int sscanf(const char *s, const char *format, ...);
PARROT_API int vfprintf(FILE *stream, const char *format, va_list arg);
PARROT_API int vfscanf(FILE *stream, const char *format, va_list arg);
PARROT_API int vprintf(const char *format, va_list arg);
PARROT_API int vscanf(const char *format, va_list arg);
PARROT_API int vsnprintf(char *s, size_t n, const char *format, va_list arg);
PARROT_API int vsprintf(char *s, const char *format, va_list arg);
PARROT_API int vsscanf(const char *s, const char *format, va_list arg);

PARROT_API int fgetc(FILE *stream);
PARROT_API char *fgets(char *restrict s, int n, FILE *restrict stream);
PARROT_API int fputc(int c, FILE *stream);
PARROT_API int fputs(const char *s, FILE *stream);
PARROT_API int getc(FILE *stream);
PARROT_API int getchar(void);
PARROT_API char *gets(char *s);
PARROT_API int putc(int c, FILE *stream);
PARROT_API int putchar(int c);
PARROT_API int puts(const char *s);
PARROT_API int ungetc(int c, FILE *stream);

PARROT_API size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
PARROT_API size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
PARROT_API int fgetpos(FILE *stream, fpos_t *pos);
PARROT_API int fseek(FILE *stream, long offset, int whence);
PARROT_API int fsetpos(FILE *stream, const fpos_t *pos);
PARROT_API long int ftell(FILE *stream);
PARROT_API void rewind(FILE *stream);
PARROT_API void clearerr(FILE *stream);
PARROT_API int feof(FILE *stream);
PARROT_API int ferror(FILE *stream);

PARROT_API void perror(const char *s);

#endif // __SRC_PARROT_PLATFORM_N64_LIBC_STDIO_H_