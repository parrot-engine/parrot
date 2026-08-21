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

export CFLAGS += -I$(INCLUDE_DIR) -I$(BUILD_INCLUDE_DIR) -I$(ROOT_DIR)/platform/$(PLATFORM)/include

export LIB_OUTPUT_STATIC=$(BUILD_DIR)/libparrot.a

ifeq ($(COMPILE_DB),1)
BEAR=bear -a -- 
endif

ifeq ($(COMPILE_DB),0)
BEAR=
endif

include platform/$(PLATFORM)/setup.mk

.PHONY: all src compile-db test test-build clean clean-all

all: src

src:
	$(BEAR) $(MAKE) -C src BUILD_DIR=$(BUILD_DIR)/lib "DRIVERS=$(DRIVERS)"

test: build-test
	$(MAKE) -C test BUILD_DIR=$(BUILD_DIR)/test run

build-test: $(LIB_OUTPUT_STATIC)
	$(BEAR) $(MAKE) -C test BUILD_DIR=$(BUILD_DIR)/test

$(LIB_OUTPUT_STATIC): src

clean:
	rm -r $(BUILD_DIR)

clean-all:
	rm -r $(ROOT_BUILD_DIR)
