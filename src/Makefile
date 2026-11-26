export CFLAGS += -std=c99 -Wall -Wextra -I$(ROOT_DIR)

ifeq ($(CONFIG_STRICT),1)
export CFLAGS += -pedantic-errors -Werror
endif

ifeq ($(CONFIG_DEBUG_INFO),1)
export CFLAGS += -g
export ASFLAGS += -g
endif

ifeq ($(CONFIG_OPTIMIZE),1)
export CFLAGS += $(PLATFORM_OPTIMIZER)
endif

CONFIG_HEADER=$(BUILD_INCLUDE_DIR)/parrot/config.h

SRC_DIRS=scene video

ifeq ($(CONFIG_PROVIDE_LIBC),1)
SRC_DIRS += libc
endif

SRC_DIRS += $(patsubst %, drivers/%, $(CONFIG_DRIVERS))

SRCS=$(shell find $(SRC_DIRS) $(CORE_DIR)/src -type f -name *.c)
OBJS=$(patsubst %, $(BUILD_DIR)/src/%.o, $(SRCS))

SRCS := $(patsubst $(CURDIR)/%, %, $(abspath $(SRCS)))

.PHONY: all
all: $(OUTPUT_STATIC)

$(OUTPUT_STATIC): $(OBJS)
	$(AR) rcs $@ $(OBJS)

$(BUILD_DIR)/src/%.o: % $(CONFIG_HEADER)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(CONFIG_HEADER):
	mkdir -p $(dir $@)
	$(PY) $(TOOLS_DIR)/generate_config.py $@
