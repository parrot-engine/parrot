MODULE=video
OUTPUT_NAME=$(MODULE)

include $(UTIL_MAKE)
include $(PLATFORM_MAKE)

SRCS += $(shell find . -name "*.c")

OBJS=$(patsubst %.c, $(BUILD_DIR)/src/%.c.o, $(SRCS))
DEPS=$(patsubst %.o, %.d, $(OBJS))

.PHONY: all

all: $(OUTPUT_MODULE)

$(OUTPUT_MODULE): $(BUILD_DIR) $(OBJS)
	echo $(OBJS) > $(OUTPUT_MODULE)

$(BUILD_DIR)/src/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
ifeq ($(DEBUG),0)
	$(STRIP) $@
endif

$(BUILD_DIR):
	mkdir -p $@

-include $(DEPS)