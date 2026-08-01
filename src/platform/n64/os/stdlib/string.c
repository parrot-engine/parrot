#include <stdint.h>
#include <string.h>

void *memcpy(void *restrict s1, const void *restrict s2, size_t n) {
    const uint8_t *src = (const uint8_t *)s2;
    uint8_t *dest = (uint8_t *)s1;

    for (size_t i = 0; i < n; i++) {
        dest[i] = src[i];
    }
    return dest;
}

void *memmove(void *s1, const void *s2, size_t n) {
    const uint8_t *src = (const uint8_t *)s2;
    uint8_t *dest = (uint8_t *)s1;

    if (dest < src) {
        while (n--) {
            *dest++ = *src++;
        }
    } else if (dest > src) {
        dest += n;
        src += n;
        while (n--) {
            *--dest = *--src;
        }
    }
    return dest;
}

char *strcpy(char *restrict dest, const char *restrict src) {
    memcpy(dest, src, (strlen(src) + 1) * sizeof(char));
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

char *strcat(char *restrict dest, const char *restrict src) {
    memcpy(&dest[strlen(dest)], src, (strlen(src) + 1) * sizeof(char));
    return dest;
}

char *strncat(char *restrict dest, const char *restrict src, size_t n) {
    char *d = dest + strlen(dest);

    size_t i = 0;
    while (i < n && src[i] != '\0') {
        *d = src[i];
        d++;
        i++;
    }

    *d = '\0';

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *dest = (const uint8_t *)s1;
    const uint8_t *src = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (dest[i] != src[i]) {
            return (dest[i] < src[i]) ? -1 : 1;
        }
    }
    return 0;
}

int strcmp(const char *dest, const char *src) {
    while (*dest && (*dest == *src)) {
        dest++;
        src++;
    }
    return *(const uint8_t *)dest - *(const uint8_t *)src;
}

int strncmp(const char *dest, const char *src, size_t n) {
    if (n == 0) {
        return 0;
    }

    while (n-- > 0 && *dest && (*dest == *src)) {
        dest++;
        src++;
    }

    if (n == (size_t)-1) {
        return 0;
    }
    return *(const unsigned char *)dest - *(const unsigned char *)src;
}

void *memchr(const void *s, int c, size_t n) {
    const uint8_t *p = (const uint8_t *)s;
    for (size_t i = 0; i < n; i++) {
        if (p[i] == (uint8_t)c) {
            return (void *)&p[i];
        }
    }
    return NULL;
}

char *strchr(const char *s, int c) {
    while (*s != '\0') {
        if (*s == (char)c) {
            return (char *)s;
        }
        s++;
    }
    return (c == '\0') ? (char *)s : NULL;
}

size_t strcspn(const char *dest, const char *src) {
    size_t i = 0;
    while (dest[i] != '\0') {
        for (size_t j = 0; src[j] != '\0'; j++) {
            if (dest[i] == src[j]) {
                return i;
            }
        }
        i++;
    }
    return i;
}

char *strpbrk(const char *dest, const char *src) {
    while (*dest != '\0') {
        for (size_t j = 0; src[j] != '\0'; j++) {
            if (*dest == src[j]) {
                return (char *)dest;
            }
        }
        dest++;
    }
    return NULL;
}

char *strrchr(const char *s, int c) {
    const char *last = NULL;
    while (*s != '\0') {
        if (*s == (char)c) {
            last = s;
        }
        s++;
    }
    return (c == '\0') ? (char *)s : (char *)last;
}

size_t strspn(const char *dest, const char *src) {
    size_t i = 0;
    while (dest[i] != '\0') {
        size_t j = 0;
        while (src[j] != '\0' && src[j] != dest[i]) {
            j++;
        }
        if (src[j] == '\0') {
            return i;
        }
        i++;
    }
    return i;
}

char *strstr(const char *dest, const char *src) {
    if (*src == '\0') {
        return (char *)dest;
    }
    size_t src_len = strlen(src);
    while (*dest != '\0') {
        if (*dest == *src && memcmp(dest, src, src_len) == 0) {
            return (char *)dest;
        }
        dest++;
    }
    return NULL;
}

char *strtok(char *restrict dest, const char *restrict src) {
    static char *saved;
    if (dest != NULL) {
        saved = dest;
    }
    if (saved == NULL) {
        return NULL;
    }
    saved += strspn(saved, src);
    if (*saved == '\0') {
        saved = NULL;
        return NULL;
    }
    char *token = saved;
    saved += strcspn(saved, src);
    if (*saved != '\0') {
        *saved++ = '\0';
    } else {
        saved = NULL;
    }
    return token;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *dest = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        dest[i] = c;
    }
    return s;
}

char *strerror(int errnum) {
    (void)errnum;
    return NULL;
}

size_t strlen(const char *s) {
    size_t size = 0;
    while (*s++) {
        size++;
    }
    return size;
}
