#include "src/platform/n64/os/libc/stdlib.malloc.h"
#include "parrot/core/util.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAGIC 0x6064454E
#define ALIGNMENT 8

#define PARROT_CHECK_HEAP() PARROT_FAIL_NULL_MSG(heap, "Attempt to use uninitialized heap")

extern uint8_t Parrot_heap_start_addr;

typedef struct AllocBlock {
    uint32_t magic;

    struct AllocBlock *prev;
    struct AllocBlock *next;

    bool free;

    size_t size;
    uint8_t data[];
} AllocBlock;

#define AllocBlock_size(x) (sizeof(AllocBlock) + ((AllocBlock *)x)->size)

static AllocBlock *heap = NULL;

static size_t alloc_align_up(size_t n) {
    return (n + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

void *calloc(size_t nmemb, size_t size) {
    PARROT_CHECK_HEAP();

    void *ptr = malloc(nmemb * size);
    PARROT_RET_COND_V(!ptr, NULL);
    return memset(ptr, 0, nmemb * size);
}

void free(void *ptr) {
    PARROT_CHECK_HEAP();

    PARROT_RET_COND(!ptr);

    AllocBlock *block = (AllocBlock *)ptr - 1;
    PARROT_FAIL_COND_FMT(block->magic != MAGIC, "Attempt to free non-heap pointer at %p", ptr);

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
    PARROT_CHECK_HEAP();

    size = alloc_align_up(size);

    AllocBlock *block = heap;

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

            new_block->magic = MAGIC;
            new_block->size = remaining_size - sizeof(AllocBlock);
            new_block->free = true;

            new_block->prev = block;
            new_block->next = block->next;

            block->next = new_block;
        }
    }

    block->free = false;
    return block->data;
}

void *realloc(void *ptr, size_t size) {
    PARROT_CHECK_HEAP();

    void *new_ptr = malloc(size);

    if (ptr && new_ptr) {
        AllocBlock *ptr_block = ((AllocBlock *)ptr - 1);
        PARROT_FAIL_COND_FMT(ptr_block->magic != MAGIC, "Attempt to reallocate non-heap pointer at %p", ptr);

        size_t ptr_size = ptr_block->size;
        size_t copy_size = ptr_size > size ? size : ptr_size;

        memcpy(new_ptr, ptr, copy_size);
        free(ptr);
    }
    return new_ptr;
}

void Parrot_os_heap_init(size_t memory_size) {
    PARROT_FAIL_COND_MSG(heap, "Attempt to initialize already initialized heap");

    heap = (AllocBlock *)&Parrot_heap_start_addr;
    memset(heap, 0, sizeof(AllocBlock));

    void *mem_end = (uint8_t *)0x80000000 + memory_size;

    heap->magic = MAGIC;

    heap->size = (uintptr_t)mem_end - (uintptr_t)heap;
    heap->free = true;
}