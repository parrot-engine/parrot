SRCS=$(shell find . -name "*.c") $(shell find . -name "*.s")
OBJS=$(patsubst %, $(BUILD_DIR)/src/%.o, $(SRCS))

all: $(OBJS)

include $(ROOT_DIR)/make/os-section.mk

$(BUILD_DIR)/src/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -S -o $@.S $<
	$(CC) $(CFLAGS) -c -o $@.tmp $<
	$(OSFILE) $@.tmp $@

include $(ROOT_DIR)/make/basic-as-target.mk
include $(ROOT_DIR)/make/cc-deps.mk
