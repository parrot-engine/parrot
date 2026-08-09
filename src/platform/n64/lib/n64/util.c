#include "parrot/platform/n64/util.h"
#include "parrot/core/util.h"

#define ICACHE_LINE_SIZE (32)
#define DCACHE_LINE_SIZE (16)

#define ICACHE_OP(op, p_addr, size)                                                                                     \
    do {                                                                                                                \
        uint8_t *aligned_addr = (uint8_t *)(uintptr_t)PARROT_ALIGN_DOWN(p_addr, (uint32_t)ICACHE_LINE_SIZE);            \
        size = PARROT_ALIGN_UP(size, ICACHE_LINE_SIZE);                                                                 \
                                                                                                                        \
        for (size_t i = 0; i < size + ICACHE_LINE_SIZE; i += ICACHE_LINE_SIZE) {                                        \
            __asm__ volatile("cache %0, (%1)" ::"i"(op), "r"(aligned_addr + i));                                        \
        }                                                                                                               \
        PARROT_N64_MSYNC();                                                                                             \
    } while (0)

#define DCACHE_OP(op, p_addr, size)                                                                                     \
    do {                                                                                                                \
        uint8_t *aligned_addr = (uint8_t *)(uintptr_t)PARROT_ALIGN_DOWN(p_addr, (uint32_t)DCACHE_LINE_SIZE);            \
        size = PARROT_ALIGN_UP(size, DCACHE_LINE_SIZE);                                                                 \
                                                                                                                        \
        for (size_t i = 0; i < size + DCACHE_LINE_SIZE; i += DCACHE_LINE_SIZE) {                                        \
            __asm__ volatile("cache %0, (%1)" ::"i"(op), "r"(aligned_addr + i));                                        \
        }                                                                                                               \
        PARROT_N64_MSYNC();                                                                                             \
    } while (0)

void Parrot_n64_invalidiate_instruction_cache(const void *addr, size_t size) {
    ICACHE_OP(/* Hit_Invalidate */ 4, (uint32_t)(uintptr_t)addr, size);
}

void Parrot_n64_invalidiate_data_cache(const void *addr, size_t size) {
    DCACHE_OP(/* Hit_Invalidate */ 0x11, (uint32_t)(uintptr_t)addr, size);
}

void Parrot_n64_writeback_invalidiate_data_cache(const void *addr, size_t size) {
    DCACHE_OP(/* Hit_Write_Back_Invalidate */ 0x15, (uint32_t)(uintptr_t)addr, size);
}
