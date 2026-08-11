SRCS=$(shell find . -name "*.c")
OBJS=$(patsubst %, $(BUILD_DIR)/src/%.o, $(SRCS))

RUNNER=exec
OUTPUT=$(BUILD_DIR)/test

CFLAGS += -g -std=c99

.PHONY: all run $(OUTPUT)

all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIB_OUTPUT_STATIC) $(LDLIBS)

run: $(OUTPUT)
	$(RUNNER) $(OUTPUT)

include $(ROOT_DIR)/make/cc-deps.mk
include $(ROOT_DIR)/make/basic-targets.mk
