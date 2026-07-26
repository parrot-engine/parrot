export CC=$(ARCH)-linux-gnu-gcc
export AR=$(ARCH)-linux-gnu-gcc-ar
export STRIP=strip

ifeq ($(OPTIMIZE),1)
export CFLAGS += -O2
endif

ifeq ($(DEBUG_INFO),1)
export CFLAGS += -g
endif
