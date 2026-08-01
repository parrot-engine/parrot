#include "parrot/core/util.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ALLOC_MAGIC 0x6055143A

#define ALLOC_ALIGNMENT 8

extern uint8_t Parrot_heap_start_addr;

static uint32_t Parrot_rng_seed = 0;

static uint8_t *Parrot_heap_start;
static uint8_t *Parrot_heap_end;
static size_t Parrot_heap_size;

typedef struct AllocBlock {
    uint32_t magic;

    struct AllocBlock *prev;
    struct AllocBlock *next;

    bool free;
    size_t size;
} AllocBlock;

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

static size_t AllocBlock_size(const AllocBlock *self) {
    return sizeof(AllocBlock) + self->size;
}

static void *AllocBlock_ptr(AllocBlock *self) {
    return (void *)(self + 1);
}

static size_t alloc_align_up(size_t n) {
    return (n + (ALLOC_ALIGNMENT - 1)) & ~(ALLOC_ALIGNMENT - 1);
}

void *calloc(size_t nmemb, size_t size) {
    void *ptr = malloc(nmemb * size);
    PARROT_RET_COND_V(!ptr, NULL);
    return memset(ptr, 0, nmemb * size);
}

void free(void *ptr) {
    PARROT_RET_COND(!ptr);

    AllocBlock *block = (AllocBlock *)ptr - 1;
    PARROT_FAIL_COND_FMT(block->magic != ALLOC_MAGIC, "Attempt to free non-heap pointer at %p", ptr);

    PARROT_FAIL_COND_FMT(block->free, "Double free at %p", ptr);
    block->free = true;

    while (block->prev && block->prev->free) {
        block = block->prev;
    }

    while (block->next && block->next->free) {
        AllocBlock *new_next_block = block->next->next;

        block->size += AllocBlock_size(block->next);

        if (new_next_block) {
            new_next_block->prev = block;
        }
        block->next = new_next_block;
    }
}

void *malloc(size_t size) {
    size = alloc_align_up(size);

    AllocBlock *block = (AllocBlock *)Parrot_heap_start;

    while (block->size < size || !block->free) {
        block = block->next;
        PARROT_RET_COND_V(!block, NULL);
    }

    if (block->size > size) {
        size_t remaining_size = block->size - size;
        if (remaining_size > sizeof(AllocBlock)) {
            block->size = size;

            AllocBlock *new_block = (AllocBlock *)((uintptr_t)block + AllocBlock_size(block));
            memset(new_block, 0, sizeof(AllocBlock));

            new_block->magic = ALLOC_MAGIC;
            new_block->size = remaining_size - sizeof(AllocBlock);
            new_block->free = true;

            new_block->prev = block;
            new_block->next = block->next;

            block->next = new_block;
        }
    }

    block->free = false;
    return AllocBlock_ptr(block);
}

void *realloc(void *ptr, size_t size) {
    void *new_ptr = malloc(size);

    if (ptr && new_ptr) {
        AllocBlock *ptr_block = ((AllocBlock *)ptr - 1);
        PARROT_FAIL_COND_FMT(ptr_block->magic != ALLOC_MAGIC, "Attempt to reallocate non-heap pointer at %p", ptr);

        size_t ptr_size = ptr_block->size;
        size_t copy_size = ptr_size > size ? size : ptr_size;

        memcpy(new_ptr, ptr, copy_size);
        free(ptr);
    }
    return new_ptr;
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

void Parrot_libc_stdlib_init(uint32_t random, size_t memory_size_bytes) {
    Parrot_rng_seed = random;

    Parrot_heap_start = &Parrot_heap_start_addr;
    Parrot_heap_end = (void *)((uintptr_t)0x80000000 + memory_size_bytes);
    Parrot_heap_size = ((uintptr_t)Parrot_heap_end) - ((uintptr_t)Parrot_heap_start);

    AllocBlock *block = (AllocBlock *)Parrot_heap_start;
    memset(block, 0, sizeof(AllocBlock));

    block->magic = ALLOC_MAGIC;
    block->free = true;
    block->size = Parrot_heap_size - sizeof(AllocBlock);
}