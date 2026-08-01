#ifndef __SRC_PARROT_SRC_PLATFORM_N64_OS_BOOT_DATA_H_
#define __SRC_PARROT_SRC_PLATFORM_N64_OS_BOOT_DATA_H_

#include <stdint.h>

typedef uint8_t LibdragonBootDataROMType;
#define LibdragonBootDataROMType_CART ((LibdragonBootDataROMType)0)
#define LibdragonBootDataROMType_64DD ((LibdragonBootDataROMType)1)

typedef uint8_t LibdragonBootDataTVType;
#define LibdragonBootDataTVType_PAL ((LibdragonBootDataTVType)0)
#define LibdragonBootDataTVType_NTSC ((LibdragonBootDataTVType)1)
#define LibdragonBootDataTVType_MPAL ((LibdragonBootDataTVType)2)

typedef uint8_t LibdragonBootDataConsoleType;
#define LibdragonBootDataConsoleType_N64 ((LibdragonBootDataConsolType)0)
#define LibdragonBootDataConsoleType_IQUE ((LibdragonBootDataConsoleType)1)

typedef struct PARROT_PACKED {
    uint32_t avaliable_memory_bytes;
    uint32_t random_seed;

    LibdragonBootDataROMType rom_type;
    LibdragonBootDataTVType tv_type;
    LibdragonBootDataConsoleType console_type;
} LibdragonBootData;

#endif // __SRC_PARROT_SRC_PLATFORM_N64_OS_BOOT_DATA_H_