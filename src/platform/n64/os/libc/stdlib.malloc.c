#include "src/platform/n64/os/libc/stdlib.malloc.h"
#include "parrot/core/util.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAGIC 0x6064454E
#define ALIGNMENT 16

#define PARROT_CHECK_HEAP() PARROT_FAIL_NULL_MSG(heap, "Attempt to use uninitialized heap")

extern uint8_t Parrot_heap_start_addr;

typedef struct {
    uint32_t magic;

    struct AllocBlock *prev;
    struct AllocBlock *next;

    bool free;

    size_t size;
} AllocBlockMeta;

typedef struct AllocBlock {
    AllocBlockMeta meta;
    uint8_t padding[PARROT_ALIGN_UP(sizeof(AllocBlockMeta), ALIGNMENT) - sizeof(AllocBlockMeta)];
    uint8_t data[];
} AllocBlock;

#define AllocBlock_size(x) (sizeof(AllocBlock) + ((AllocBlock *)x)->meta.size)

static AllocBlock *heap = NULL;

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
    PARROT_FAIL_COND_FMT(block->meta.magic != MAGIC, "Attempt to free non-heap pointer at %p", ptr);

    PARROT_FAIL_COND_FMT(block->meta.free, "Double free at %p", ptr);
    block->meta.free = true;

    while (block->meta.prev && block->meta.prev->meta.free) {
        block = block->meta.prev;
    }

    while (block->meta.next && block->meta.next->meta.free) {
        AllocBlock *new_next_block = block->meta.next->meta.next;

        block->meta.size += AllocBlock_size(block->meta.next);

        if (new_next_block) {
            new_next_block->meta.prev = block;
        }
        block->meta.next = new_next_block;
    }
}

void *malloc(size_t size) {
    PARROT_CHECK_HEAP();

    size = PARROT_ALIGN_UP(size, ALIGNMENT);

    AllocBlock *block = heap;

    while (block->meta.size < size || !block->meta.free) {
        block = block->meta.next;
        PARROT_RET_COND_V(!block, NULL);
    }

    if (block->meta.size > size) {
        size_t remaining_size = block->meta.size - size;
        if (remaining_size > sizeof(AllocBlock)) {
            block->meta.size = size;

            AllocBlock *new_block = (AllocBlock *)((uint8_t *)block + AllocBlock_size(block));
            memset(new_block, 0, sizeof(AllocBlock));

            new_block->meta.magic = MAGIC;
            new_block->meta.size = remaining_size - sizeof(AllocBlock);
            new_block->meta.free = true;

            new_block->meta.prev = block;
            new_block->meta.next = block->meta.next;

            if (new_block->meta.next) {
                new_block->meta.next->meta.prev = new_block;
            }

            block->meta.next = new_block;
        }
    }

    block->meta.free = false;
    return block->data;
}

void *realloc(void *ptr, size_t size) {
    PARROT_CHECK_HEAP();

    void *new_ptr = malloc(size);

    if (ptr && new_ptr) {
        AllocBlock *ptr_block = ((AllocBlock *)ptr - 1);
        PARROT_FAIL_COND_FMT(ptr_block->meta.magic != MAGIC, "Attempt to reallocate non-heap pointer at %p", ptr);

        size_t ptr_size = ptr_block->meta.size;
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

    heap->meta.magic = MAGIC;

    heap->meta.size = (uintptr_t)((uint8_t *)mem_end - (uint8_t *)heap) - sizeof(AllocBlock);
    heap->meta.free = true;
}
