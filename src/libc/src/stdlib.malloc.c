#include "parrot/config.h"
#include "parrot/core/util.h"
#include "parrot/libc/yield.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PARROT_CHECK_HEAP() PARROT_FAIL_NULL_MSG(heap_begin, "Attempt to use uninitialized heap")

typedef struct AllocBlock {
    struct AllocBlock *prev;
    struct AllocBlock *next;

    bool free;
    uint64_t segment;

    size_t size;
} AllocBlock;

static AllocBlock *heap_begin = NULL;
static uint64_t heap_segment = 0;

#define PAGE_SIZE (4096)

#ifdef PAGE_SIZE
static void *alloc_rw_pages(size_t page_count) {
    (void)page_count;
    return NULL;
}
#endif

void *calloc(size_t nmemb, size_t size) {
    PARROT_CHECK_HEAP();

    void *ptr = malloc(nmemb * size);
    PARROT_RET_COND_V(!ptr, NULL);

    Parrot_libc_yield();
    return memset(ptr, 0, nmemb * size);
}

void free(void *ptr) {
    PARROT_CHECK_HEAP();

    PARROT_RET_COND(!ptr);

    AllocBlock *block = PARROT_META(sizeof(AllocBlock), ptr);
    PARROT_FAIL_COND_FMT(block->free, "Double free at %p", ptr);
    block->free = true;

    while (block->prev && block->prev->free && block->prev->segment == block->segment) {
        block = block->prev;
    }

    while (block->next && block->next->free && block->next->segment == block->segment) {
        AllocBlock *new_next_block = block->next->next;

        block->size += PARROT_META_SIZE(sizeof(AllocBlock), block->next->size);

        if (new_next_block) {
            new_next_block->prev = block;
        }
        block->next = new_next_block;
    }

    Parrot_libc_yield();
}

void *malloc(size_t size) {
    PARROT_CHECK_HEAP();

    size = PARROT_ALIGN_UP(size, 16);

    AllocBlock *block = heap_begin;

    while (!block->free || block->size < size) {
        AllocBlock *prev_block = block;
        block = block->next;

        if (!block) {
#ifdef PAGE_SIZE
            size_t page_count = (size + PARROT_META_SIZE(sizeof(AllocBlock), 0) + PAGE_SIZE - 1) / PAGE_SIZE;
            size_t alloc_size = page_count * 4096;

            AllocBlock *new_block = alloc_rw_pages(page_count);
            PARROT_RET_COND_V(!new_block, NULL);
            memset(new_block, 0, sizeof(*new_block));

            new_block->size = alloc_size - PARROT_META_SIZE(sizeof(AllocBlock), 0);
            new_block->free = true;
            new_block->segment = heap_segment++;

            prev_block->next = new_block;
            new_block->prev = prev_block;
            new_block->next = NULL;

            block = new_block;
#else
            (void)prev_block;
            return NULL;
#endif
        }
    }

    if (block->size > size) {
        size_t remaining_size = block->size - size;
        if (remaining_size > PARROT_META_SIZE(sizeof(AllocBlock), 0)) {
            block->size = size;

            AllocBlock *new_block = (AllocBlock *)((uint8_t *)block + PARROT_META_SIZE(sizeof(AllocBlock), block->size));
            memset(new_block, 0, sizeof(*new_block));

            new_block->size = remaining_size - PARROT_META_SIZE(sizeof(AllocBlock), 0);
            new_block->free = true;

            new_block->prev = block;
            new_block->next = block->next;

            if (new_block->next) {
                new_block->next->prev = new_block;
            }

            block->next = new_block;
        }
    }

    block->free = false;
    Parrot_libc_yield();
    return PARROT_META_USER(sizeof(AllocBlock), block);
}

void *realloc(void *ptr, size_t size) {
    PARROT_CHECK_HEAP();

    void *new_ptr = malloc(size);

    if (ptr && new_ptr) {
        AllocBlock *ptr_block = PARROT_META(sizeof(AllocBlock), ptr);

        size_t ptr_size = ptr_block->size;
        size_t copy_size = ptr_size > size ? size : ptr_size;

        memcpy(new_ptr, ptr, copy_size);
        free(ptr);
    }

    Parrot_libc_yield();
    return new_ptr;
}

void Parrot_libc_init_heap(void) {
    PARROT_FAIL_COND_MSG(heap_begin, "Attempt to initialize already initialized heap");

#ifdef PAGE_SIZE
    size_t heap_size = PAGE_SIZE;
    heap_begin = alloc_rw_pages(1);
#endif

    PARROT_FAIL_NULL_MSG(heap_begin, "Failed to initialize heap");

    memset(heap_begin, 0, sizeof(AllocBlock));

    heap_begin->size = heap_size - PARROT_META_SIZE(sizeof(AllocBlock), 0);
    heap_begin->free = true;
    heap_begin->segment = heap_segment++;
}
