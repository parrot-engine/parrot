SRCS=$(shell find . -name "*.c")
SRCS := $(patsubst $(CURDIR)/%, %, $(abspath $(SRCS)))

OBJS=$(patsubst %, $(BUILD_DIR)/src/%.o, $(SRCS))

RUNNER=exec

OUTPUT=$(BUILD_DIR)/test

CFLAGS += -g -std=c99

.PHONY: all
all: $(OUTPUT)

$(OUTPUT): $(OBJS) $(OUTPUT_STATIC)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(OUTPUT_STATIC) $(LDLIBS)

$(BUILD_DIR)/src/%.o: %
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
