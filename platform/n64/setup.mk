export CC=mips64-elf-gcc
export AR=mips64-elf-ar
export STRIP=mips64-elf-strip

export CFLAGS += -march=mips3 -mhard-float -mfp64 -mfix4300 -ffreestanding

ifeq ($(N64_PROVIDE_OS),1)
export CFLAGS += -nostdinc -isystem$(ROOT_DIR)/platform/n64/libc
export LDFLAGS += -nostdlib -T$(ROOT_DIR)/platform/n64/linker.ld
endif
