export CC=mips64-elf-gcc
export AR=mips64-elf-ar
export STRIP=mips64-elf-strip

export CFLAGS += -march=vr4300 -mhard-float -mfp64 -mfix4300
export LDFLAGS += -nostdlib

ifeq ($(OPTIMIZE),1)
export CFLAGS += -Os
endif

ifeq ($(DEBUG_INFO),1)
export CFLAGS += -g
endif
