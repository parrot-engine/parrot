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

#define LibdragonBootData_avaliable_memory_bytes (*(volatile uint32_t *)0xA4000000)
#define LibdragonBootData_avaliable_random (*(volatile uint32_t *)0xA4000004)

#define LibdragonBootData_rom_type (*(LibdragonBootDataROMType *)0xA4000008)
#define LibdragonBootData_tv_type (*(LibdragonBootDataTVType *)0xA4000009)
#define LibdragonBootData_console_type (*(LibdragonBootDataConsoleType *)0xA400000B)

#endif // __SRC_PARROT_SRC_PLATFORM_N64_OS_BOOT_DATA_H_
