SRCS=$(shell find . -name "*.c")
OBJS=$(patsubst %, $(BUILD_DIR)/src/%.o, $(SRCS))

.PHONY: all
all: $(OBJS)

include $(ROOT_DIR)/make/basic-targets.mk
include $(ROOT_DIR)/make/cc-deps.mk
