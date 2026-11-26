export DEFAULT=__default__

export ROOT_DIR=$(shell pwd)

export MAKE=make
export PY=python3

CONFIG_FILE=

ifneq ($(CONFIG_FILE),)
include $(CONFIG_FILE)
endif

include config.mk

export CORE_DIR=$(ROOT_DIR)/core
export BUILD_DIR=$(ROOT_DIR)/build/$(CONFIG_BUILD_DIR_NAME)
export TOOLS_DIR=$(ROOT_DIR)/tools

export BUILD_INCLUDE_DIR=$(BUILD_DIR)/include

export CFLAGS += $(CONFIG_EXTRA_CFLAGS) $(CONFIG_EXTRA_CLDFLAGS)
export ASFLAGS += $(CONFIG_EXTRA_ASFLAGS)
export LDFLAGS += $(CONFIG_EXTRA_LDFLAGS) $(CONFIG_EXTRA_CLDFLAGS)

export CFLAGS += -I$(ROOT_DIR)/include
export CFLAGS += -I$(CORE_DIR)/include
export CFLAGS += -I$(BUILD_INCLUDE_DIR)

ifeq ($(CONFIG_PROVIDE_LIBC),1)
export CFLAGS += -I$(ROOT_DIR)/include/parrot/libc
endif

ifeq ($(CONFIG_PLATFORM),linux)
export CC=$(CONFIG_ARCH)-linux-gnu-gcc
export AR=$(CONFIG_ARCH)-linux-gnu-gcc-ar
export OBJCOPY=$(CONFIG_ARCH)-linux-gnu-gcc-objcopy
export STRIP=strip

export LDLIBS += -lX11 -lXext -lGL -lm

export PLATFORM_OPTIMIZER = -O2

export PLATFORM_UNIX = 1
export PLATFORM_MULTIARCH = 1
export PLATFORM_HAS_LIBC = 1

override CONFIG_DRIVERS := $(subst default,x11 gl11,$(CONFIG_DRIVERS))
endif

include defaults.mk

export OUTPUT_STATIC=$(BUILD_DIR)/libparrot
export CORE_OUTPUT_STATIC=$(BUILD_DIR)/libparrotcore

export OUTPUT_STATIC := $(OUTPUT_STATIC).a
export CORE_OUTPUT_STATIC := $(CORE_OUTPUT_STATIC).a

.PHONY: all
all: $(OUTPUT_STATIC)

$(OUTPUT_STATIC): src

.PHONY: src
src:
	$(MAKE) -C src "CONFIG_DRIVERS=$(CONFIG_DRIVERS)"

.PHONY: test
test: src
	$(MAKE) -C test BUILD_DIR=$(BUILD_DIR)/test

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

.PHONY: clean-all
clean-all:
	rm -r $(ROOT_BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
