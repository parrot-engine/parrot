SRCS=$(shell find $(CORE_DIR)/src -name "*.c")
OBJS=$(patsubst %, $(BUILD_DIR)/src/%.o, $(SRCS))

all: $(OBJS)

include $(ROOT_DIR)/make/os-section.mk

$(BUILD_DIR)/src/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@.tmp $<
ifeq ($(PLATFORM),n64)
ifeq ($(N64_PROVIDE_OS),1)
	$(OSFILE) $@.tmp $@
else
	cp $@.tmp $@
endif
else
	cp $@.tmp $@
endif

include $(ROOT_DIR)/make/basic-as-target.mk
include $(ROOT_DIR)/make/cc-deps.mk
