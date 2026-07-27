export CC=$(ARCH)-linux-gnu-gcc
export AR=$(ARCH)-linux-gnu-gcc-ar
export STRIP=strip

export PLATFORM_HAS_OS ?= 1
export PLATFORM_UNIX ?= 1

export LDFLAGS += -lX11 -lGL

include $(ROOT_DIR)/make/platform_base/modern/setup.mk
include $(ROOT_DIR)/make/platform_base/os/setup.mk
