export ROOT_DIR ?= $(shell pwd)

CONFIG_FILE=$(ROOT_DIR)/config.mk
ifneq ($(CONFIG_FILE),)
-include $(CONFIG_FILE)
endif

include $(ROOT_DIR)/default_config.mk

export MAKE=make

export NAME=parrot

export PLATFORM_ROOT_DIR=$(ROOT_DIR)/platform
export PLATFORM_DIR=$(PLATFORM_ROOT_DIR)/$(PLATFORM)

export SRC_DIR=$(ROOT_DIR)/src

BUILD_DIR_NAME=$(PLATFORM)

ifeq ($(DEBUG_INFO),1)
BUILD_DIR_NAME := $(BUILD_DIR_NAME)+symbols
endif

ifeq ($(OPTIMIZE),1)
BUILD_DIR_NAME := $(BUILD_DIR_NAME)+optimized
endif

export BUILD_ROOT_DIR=$(ROOT_DIR)/build
export RAW_BUILD_DIR=$(BUILD_ROOT_DIR)/$(BUILD_DIR_NAME)

export UTIL_MAKE=$(ROOT_DIR)/make/util.mk
export PLATFORM_MAKE=$(PLATFORM_DIR)/setup.mk

export PRIVATE_CFLAGS=-std=c99 -pedantic-errors -I$(ROOT_DIR) -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables
export PUBLIC_CFLAGS=-I$(ROOT_DIR)/include

PUBLIC_CFLAGS += -DParrotReal=$(REAL_NUMBER_TYPE)

ifeq ($(DEBUG),1)
PUBLIC_CFLAGS += -DPARROT_DEBUG
endif

ifeq ($(BUILD_SHARED),1)
PRIVATE_CFLAGS += -fPIC
endif

BEAR=bear

.PHONY: all src compile_db clean root_clean

all: src

src:
	$(MAKE) -C $(SRC_DIR)

compile_db:
	$(BEAR) -- $(MAKE) -B

clean:
	rm -r $(RAW_BUILD_DIR)

root_clean:
	rm -r $(BUILD_ROOT_DIR)