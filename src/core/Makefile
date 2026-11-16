SRCS=$(shell find $(CORE_DIR)/src -name "*.c")
OBJS=$(patsubst %, $(BUILD_DIR)/src/%.o, $(SRCS))

ifeq ($(PROVIDE_LIBC),1)
SRCS += $(shell find libc -name "*.c")
endif

all: $(OBJS)

include $(ROOT_DIR)/make/os-section.mk

$(BUILD_DIR)/src/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

include $(ROOT_DIR)/make/basic-as-target.mk
include $(ROOT_DIR)/make/cc-deps.mk
