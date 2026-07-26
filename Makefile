export DEFAULT=__default__

export ROOT_DIR=$(shell pwd)

export MAKE=make

CONFIG_FILE=

ifneq ($(CONFIG_FILE),)
include $(CONFIG_FILE)
endif

include config.default.mk

export ROOT_BUILD_DIR=$(ROOT_DIR)/build
export BUILD_DIR=$(ROOT_BUILD_DIR)/$(BUILD_DIR_NAME)

export INCLUDE_DIR=$(ROOT_DIR)/include
export BUILD_INCLUDE_DIR=$(BUILD_DIR)/include

export CFLAGS += -I$(INCLUDE_DIR) -I$(BUILD_INCLUDE_DIR)

include platform/$(PLATFORM)/setup.mk

.PHONY: all src clean clean-all

all: src

src:
	$(MAKE) -C src

clean:
	rm -r $(BUILD_DIR)

clean-all:
	rm -r $(ROOT_BUILD_DIR)
