include $(PLATFORM_DIR)/default_config.mk
-include $(PLATFORM_DIR)/config.mk

CFLAGS += $(PUBLIC_CFLAGS) $(PRIVATE_CFLAGS)
CFLAGS += -DPARROT_PLATFORM_N64
CFLAGS += $(SLOW_MEMORY_CFLAGS)

CFLAGS += -march=vr4300
CFLAGS += -mhard-float -mfp64

ifeq ($(DEBUG_INFO),1)
CFLAGS += -g
endif

ifeq ($(OPTIMIZE),1)
CFLAGS += $(OPTIMIZER)
endif

OUTPUT_STATIC=$(BUILD_DIR)/$(OUTPUT_NAME).a

OUTPUT_MODULE_NAME=module.txt
OUTPUT_MODULE=$(BUILD_DIR)/$(OUTPUT_MODULE_NAME)

ifeq ($(BUILD_SHARED),1)
$(error Cannot build shared N64 libraries)
endif

ifeq ($(MODULE),)
ifeq ($(BUILD_N64_OS),1)
SRCS += $(shell find $(PLATFORM_DIR)/src/os -name "*.c")
SRCS += $(shell find $(PLATFORM_DIR)/src/os -name "*.s")
endif
endif