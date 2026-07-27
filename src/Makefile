export CFLAGS += -std=c99 -Wall -Wextra -pedantic-errors -MMD -MP -I$(ROOT_DIR) -I$(shell pwd)/include

CONFIG_HEADER=$(BUILD_INCLUDE_DIR)/parrot/config.h
CONFIG_HEADER_GUARD_NAME := _$(shell openssl rand -hex 32)_PARROT_CONFIG_H_

MODULE_BUILD_DIRS=
MODULE_TARGETS=

SRCS=$(shell echo *.c)
OBJS=$(patsubst %, $(BUILD_DIR)/src/%.o, $(SRCS))

.PHONY: all

all: $(LIB_OUTPUT_STATIC)

define ADD_MODULE
.PHONY: $(1)_module
$(1)_module: $(CONFIG_HEADER)
	$(MAKE) -C $(1) BUILD_DIR=$(BUILD_DIR)/$(1)

MODULE_BUILD_DIRS += $(BUILD_DIR)/$(1)
MODULE_TARGETS += $(1)_module
endef

$(eval $(call ADD_MODULE,core))
$(eval $(call ADD_MODULE,scene))

ifeq ($(VIDEO_MODULE_ENABLE),1)
$(eval $(call ADD_MODULE,video))
endif

$(eval $(call ADD_MODULE,platform))

$(LIB_OUTPUT_STATIC): $(CONFIG_HEADER) $(MODULE_TARGETS) $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS) $(shell find $(MODULE_BUILD_DIRS) -name '*.o' || true)

$(CONFIG_HEADER):
	mkdir -p $(dir $@)

	echo "#ifndef $(CONFIG_HEADER_GUARD_NAME)" > $(CONFIG_HEADER)
	echo "#define $(CONFIG_HEADER_GUARD_NAME)" >> $(CONFIG_HEADER)

ifeq ($(PLATFORM_HAS_OS),1)
	echo "#define PARROT_PLATFORM_HAS_OS" >> $(CONFIG_HEADER)
endif

ifeq ($(PLATFORM_UNIX),1)
	echo "#define PARROT_PLATFORM_UNIX" >> $(CONFIG_HEADER)
endif

	echo "#define PARROT_PLATFORM_$(shell echo "$(PLATFORM)" | tr 'a-z' 'A-Z')" >> $(CONFIG_HEADER)

ifeq ($(DEBUG),1)
	echo "#define PARROT_DEBUG" >> $(CONFIG_HEADER)
endif

ifeq ($(DOUBLE_PRECISION),0)
	echo "#define ParrotReal float" >> $(CONFIG_HEADER)
endif
ifeq ($(DOUBLE_PRECISION),1)
	echo "#define ParrotReal double" >> $(CONFIG_HEADER)
endif

ifeq ($(AVOID_MEMORY),1)
	echo "#define PARROT_AVOID_MEMORY" >> $(CONFIG_HEADER)
endif

	echo "#endif // $(CONFIG_HEADER_GUARD_NAME)" >> $(CONFIG_HEADER)

include $(ROOT_DIR)/make/cc-deps.mk
include $(ROOT_DIR)/make/basic-targets.mk
