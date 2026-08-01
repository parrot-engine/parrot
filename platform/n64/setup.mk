export CC=mips64-elf-gcc
export AR=mips64-elf-ar
export STRIP=mips64-elf-strip

export CFLAGS += -nostdinc -march=mips3 -mhard-float -mfp64 -mfix4300 -ffreestanding -G 0

ifeq ($(N64_PROVIDE_OS),1)
export CFLAGS += -isystem$(ROOT_DIR)/platform/n64/stdlib
export LDFLAGS += -nostdlib -T$(ROOT_DIR)/platform/n64/linker.ld -G0
endif

ifeq ($(OPTIMIZE),1)
export CFLAGS += -Os
endif

ifeq ($(DEBUG_INFO),1)
export CFLAGS += -g
endif
