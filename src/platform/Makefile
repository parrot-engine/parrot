SRCS=dummy.s
SRC_DIRS=

ifneq ($(wildcard $(PLATFORM)),)
SRC_DIRS += $(PLATFORM)
endif

ifeq ($(PLATFORM_UNIX),1)
SRC_DIRS += unix
endif

ifeq ($(PLATFORM_HAS_OS),1)
SRC_DIRS += os
endif

ifneq ($(SRC_DIRS),)
SRCS=$(shell find $(SRC_DIRS) -name "*.c") $(shell find $(SRC_DIRS) -name "*.s")
endif

OBJS=$(patsubst %, $(BUILD_DIR)/src/%.o, $(SRCS))

all: $(OBJS)

include $(ROOT_DIR)/make/cc-deps.mk
include $(ROOT_DIR)/make/basic-targets.mk

