export CC=$(ARCH)-linux-gnu-gcc
export AR=$(ARCH)-linux-gnu-gcc-ar
export OBJCOPY=$(ARCH)-linux-gnu-gcc-objcopy
export STRIP=strip

export LDLIBS += -lX11 -lXext -lGL

include $(ROOT_DIR)/make/platform_base/os/setup.mk
