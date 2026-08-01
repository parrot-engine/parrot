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

export LIB_OUTPUT_STATIC=$(BUILD_DIR)/libparrot.a

BEAR=bear

include platform/$(PLATFORM)/setup.mk

.PHONY: all src compile-db test clean clean-all

all: src

src:
	$(BEAR) -a -- $(MAKE) -C src BUILD_DIR=$(BUILD_DIR)/lib

test: $(LIB_OUTPUT_STATIC)
	$(BEAR) -a -- $(MAKE) -C test BUILD_DIR=$(BUILD_DIR)/test run

$(LIB_OUTPUT_STATIC): src

clean:
	rm -r $(BUILD_DIR)

clean-all:
	rm -r $(ROOT_BUILD_DIR)
