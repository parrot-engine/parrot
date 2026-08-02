#include <stdint.h>
#include <stdlib.h>

int Parrot_rng_seed = 1;

double atof(const char *nptr) {
    return strtold(nptr, NULL);
}

int atoi(const char *nptr) {
    return strtol(nptr, NULL, 10);
}

long int atol(const char *nptr) {
    return strtol(nptr, NULL, 10);
}

long long int atoll(const char *nptr) {
    return strtoll(nptr, NULL, 10);
}

double strtod(const char *restrict nptr, char **restrict endptr) {
    return (double)strtold(nptr, endptr);
}

float strtof(const char *restrict nptr, char **restrict endptr) {
    return (float)strtold(nptr, endptr);
}

long double strtold(const char *restrict nptr, char **restrict endptr) {
    const char *p = nptr;
    long double result = 0.0L;
    int sign = 1;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\f' || *p == '\v') {
        p++;
    }
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }
    while (*p >= '0' && *p <= '9') {
        result = result * 10.0L + (*p - '0');
        p++;
    }
    if (*p == '.') {
        p++;
        long double frac = 0.1L;
        while (*p >= '0' && *p <= '9') {
            result += (*p - '0') * frac;
            frac *= 0.1L;
            p++;
        }
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        int exp_sign = 1;
        int exp = 0;
        if (*p == '-') {
            exp_sign = -1;
            p++;
        } else if (*p == '+') {
            p++;
        }
        while (*p >= '0' && *p <= '9') {
            exp = exp * 10 + (*p - '0');
            p++;
        }
        long double power = 1.0L;
        for (int i = 0; i < exp; i++) {
            power *= 10.0L;
        }
        if (exp_sign < 0) {
            result /= power;
        } else {
            result *= power;
        }
    }
    if (endptr) {
        *endptr = (char *)p;
    }
    return result * sign;
}

long int strtol(const char *restrict nptr, char **restrict endptr, int base) {
    return (long int)strtoll(nptr, endptr, base);
}

long long int strtoll(const char *restrict nptr, char **restrict endptr, int base) {
    return (long long int)strtoull(nptr, endptr, base);
}

unsigned long int strtoul(const char *restrict nptr, char **restrict endptr, int base) {
    return (unsigned long int)strtoull(nptr, endptr, base);
}

unsigned long long int strtoull(const char *restrict nptr, char **restrict endptr, int base) {
    const char *p = nptr;
    unsigned long long int result = 0;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\f' || *p == '\v') {
        p++;
    }
    if (*p == '+') {
        p++;
    }
    if (base == 0) {
        if (*p == '0') {
            p++;
            if (*p == 'x' || *p == 'X') {
                base = 16;
                p++;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
    } else if (base == 16 && *p == '0' && (p[1] == 'x' || p[1] == 'X')) {
        p += 2;
    }
    while (*p) {
        int digit;
        if (*p >= '0' && *p <= '9') {
            digit = *p - '0';
        } else if (*p >= 'a' && *p <= 'z') {
            digit = *p - 'a' + 10;
        } else if (*p >= 'A' && *p <= 'Z') {
            digit = *p - 'A' + 10;
        } else {
            break;
        }
        if (digit >= base) {
            break;
        }
        result = result * base + digit;
        p++;
    }
    if (endptr) {
        *endptr = (char *)p;
    }
    return result;
}

int rand(void) {
    Parrot_rng_seed = (1664525 * Parrot_rng_seed + 1013904223);
    return Parrot_rng_seed;
}

void srand(unsigned int seed) {
    Parrot_rng_seed = seed;
}

void abort(void) {
    for (;;)
        ;
}

int atexit(void (*func)(void)) {
    (void)func;
    return 0;
}

void exit(int status) {
    (void)status;
    for (;;)
        ;
}

char *getenv(const char *name) {
    (void)name;
    return NULL;
}

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)) {
    for (size_t i = 0; i < nmemb; i++) {
        void *element = (uint8_t *)base + size * i;
        if (compar(element, key) == 0) {
            return element;
        }
    }
    return NULL;
}

void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)) {
    for (size_t i = 0; i < nmemb; i++) {
        for (size_t j = 0; j < nmemb - 1; j++) {
            void *a = (uint8_t *)base + size * j;
            void *b = (uint8_t *)base + size * (j + 1);
            if (compar(a, b) > 0) {
                for (size_t k = 0; k < size; k++) {
                    uint8_t tmp = ((uint8_t *)a)[k];
                    ((uint8_t *)a)[k] = ((uint8_t *)b)[k];
                    ((uint8_t *)b)[k] = tmp;
                }
            }
        }
    }
}

int abs(int j) {
    return j > 0 ? j : -j;
}

long int labs(long int j) {
    return j > 0 ? j : -j;
}

long long int llabs(long long int j) {
    return j > 0 ? j : -j;
}