SRCS=$(shell find . -name "*.c") $(shell find . -name "*.S")
OBJS=$(patsubst %, $(BUILD_DIR)/src/%.o, $(SRCS))

UCODE_OUTPUT=$(BUILD_DIR)/src/ucode/ucode.bin

ASFLAGS += -DUCODE_PATH=$(UCODE_OUTPUT)

NAKEN_ASM=naken_asm

all: $(OBJS)

$(UCODE_OUTPUT): ucode/ucode.rsp
	mkdir -p $(dir $@)
	$(NAKEN_ASM) -type bin -o $@ $<

$(BUILD_DIR)/src/./ucode/ucode.S.o: $(UCODE_OUTPUT)

include $(ROOT_DIR)/make/basic-targets.mk
include $(ROOT_DIR)/make/cc-deps.mk
