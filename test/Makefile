SRCS=$(shell find . -name "*.c")
OBJS=$(patsubst %, $(BUILD_DIR)/src/%.o, $(SRCS))

ifeq ($(PLATFORM),n64)
RUNNER=ares
else
RUNNER=exec
endif

OUTPUT=$(BUILD_DIR)/test

CFLAGS += -g -std=c99

.PHONY: all run $(OUTPUT)

all: $(OUTPUT)

$(OUTPUT): $(OBJS)
ifeq ($(PLATFORM),n64)
	$(CC) $(LDFLAGS) -o $@.elf $(OBJS) $(LIB_OUTPUT_STATIC) $(LDLIBS)
	cat ipl3_dev.z64 $@.elf > $@.z64
else
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIB_OUTPUT_STATIC) $(LDLIBS)
endif

run: $(OUTPUT)
ifeq ($(PLATFORM),n64)
	$(RUNNER) $(OUTPUT).z64
else
	$(RUNNER) $(OUTPUT)
endif

include $(ROOT_DIR)/make/cc-deps.mk
include $(ROOT_DIR)/make/basic-targets.mk
