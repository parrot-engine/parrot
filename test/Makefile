SRCS=$(shell find . -name "*.c")
OBJS=$(patsubst %, $(BUILD_DIR)/src/%.o, $(SRCS))

OUTPUT=$(BUILD_DIR)/test

.PHONY: all run $(OUTPUT)

all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CC) $(LDFLAGS) -o $(OUTPUT) $(OBJS) $(LIB_OUTPUT_STATIC)

run: $(OUTPUT)
	$(OUTPUT)

include $(ROOT_DIR)/make/cc-deps.mk
include $(ROOT_DIR)/make/basic-targets.mk
