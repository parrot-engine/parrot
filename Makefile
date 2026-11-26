export DEFAULT=__default__

export ROOT_DIR=$(shell pwd)

export MAKE=make
export PY=python3

CONFIG_FILE=

ifneq ($(CONFIG_FILE),)
include $(CONFIG_FILE)
endif

include config.mk

export CORE_DIR=core
export TOOLS_DIR=tools

export SRC_DIR=src
export INCLUDE_DIR=include
export BUILD_DIR=build/$(CONFIG_BUILD_DIR_NAME)

export BUILD_INCLUDE_DIR=$(BUILD_DIR)/include

export CFLAGS += $(CONFIG_EXTRA_CFLAGS) $(CONFIG_EXTRA_CLDFLAGS)
export ASFLAGS += $(CONFIG_EXTRA_ASFLAGS)
export LDFLAGS += $(CONFIG_EXTRA_LDFLAGS) $(CONFIG_EXTRA_CLDFLAGS)

export CFLAGS += -I$(abspath $(INCLUDE_DIR))
export CFLAGS += -I$(abspath $(CORE_DIR)/include)
export CFLAGS += -I$(abspath $(BUILD_INCLUDE_DIR))

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

export OUTPUT_STATIC=$(abspath $(BUILD_DIR)/libparrot)

export OUTPUT_STATIC := $(OUTPUT_STATIC).a

.PHONY: all
all: $(OUTPUT_STATIC)

.PHONY: test
test: $(OUTPUT_STATIC)
	$(MAKE) -C test BUILD_DIR=$(BUILD_DIR)/test

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

.PHONY: clean-all
clean-all:
	rm -r $(ROOT_BUILD_DIR)

########################################################################################################################

_CFLAGS=$(CFLAGS) -MMD -std=c99 -Wall -Wextra -I$(ROOT_DIR)

ifeq ($(CONFIG_STRICT),1)
_CFLAGS += -pedantic-errors -Werror
endif

ifeq ($(CONFIG_DEBUG_INFO),1)
_CFLAGS += -g
export ASFLAGS += -g
endif

ifeq ($(CONFIG_OPTIMIZE),1)
_CFLAGS += $(PLATFORM_OPTIMIZER)
endif

_CONFIG_HEADER=$(BUILD_INCLUDE_DIR)/parrot/config.h

_SRC_DIRS=$(SRC_DIR)/scene $(SRC_DIR)/video

ifeq ($(CONFIG_PROVIDE_LIBC),1)
_SRC_DIRS += $(SRC_DIR)/libc
endif

_SRC_DIRS += $(patsubst %, $(SRC_DIR)/drivers/%, $(CONFIG_DRIVERS))

_SRCS=$(shell find $(_SRC_DIRS) $(CORE_DIR)/src -type f -name "*.c")
_OBJS=$(patsubst %, $(BUILD_DIR)/%.o, $(_SRCS))

.PHONY: all
all: $(OUTPUT_STATIC)

$(OUTPUT_STATIC): $(_OBJS)
	echo $(_SRCS)
	$(AR) rcs $@ $(_OBJS)

$(BUILD_DIR)/%.o: % $(_CONFIG_HEADER)
	mkdir -p $(dir $@)
	$(CC) $(_CFLAGS) -c -o $@ $<

$(_CONFIG_HEADER):
	mkdir -p $(dir $@)
	$(PY) $(TOOLS_DIR)/generate_config.py $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

-include $(patsubst %, %.d, $(SRCS))
