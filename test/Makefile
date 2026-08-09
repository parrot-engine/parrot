SRCS=$(shell find . -name "*.c") $(shell find . -name "*.s")
OBJS=$(patsubst %, $(BUILD_DIR)/src/%.o, $(SRCS))


ifeq ($(PLATFORM),n64)
RUNNER=ares
OUTPUT=$(BUILD_DIR)/test.z64
else
RUNNER=exec
OUTPUT=$(BUILD_DIR)/test
endif

.PHONY: all run $(OUTPUT)

all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIB_OUTPUT_STATIC) $(LDLIBS)
ifeq ($(PLATFORM),n64)
	mv $@ $(basename $@).elf
	cat ipl3_dev.z64 $(basename $@).elf > $@
endif

run: $(OUTPUT)
	$(RUNNER) $(OUTPUT)

include $(ROOT_DIR)/make/cc-deps.mk
include $(ROOT_DIR)/make/basic-targets.mk
