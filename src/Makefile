OUTPUT_NAME=parrot

include $(UTIL_MAKE)
include $(PLATFORM_MAKE)

TARGETS=
MODULES=core

ifeq ($(MODULE_SCENE_ENABLE),1)
MODULES += scene
endif

ifeq ($(MODULE_VIDEO_ENABLE),1)
MODULES += video
endif

MODULE_TARGETS=$(patsubst %, module_%, $(MODULES))

SRCS += $(shell echo *.c)

OBJS=$(patsubst %, $(BUILD_DIR)/src/%.o, $(SRCS))
DEPS=$(patsubst %.o, %.d, $(OBJS))

ifeq ($(BUILD_SHARED),1)
TARGETS += $(OUTPUT_SHARED)
endif

ifeq ($(BUILD_STATIC),1)
TARGETS += $(OUTPUT_STATIC)
endif

.PHONY: all

all: $(TARGETS)

$(OUTPUT_SHARED): $(BUILD_DIR) $(MODULE_TARGETS) $(OBJS)
	$(CC) $(LDFLAGS) -shared -o $@ $(OBJS) $(shell cat $(patsubst %, $(BUILD_DIR)/%/$(OUTPUT_MODULE_NAME), $(MODULES)))

$(OUTPUT_STATIC): $(BUILD_DIR) $(MODULE_TARGETS) $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS) $(shell cat $(patsubst %, $(BUILD_DIR)/%/$(OUTPUT_MODULE_NAME), $(MODULES)))

$(BUILD_DIR)/src/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
ifeq ($(DEBUG),0)
	$(STRIP) $@
endif

$(BUILD_DIR)/src/%.s.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c -o $@ $<

module_%:
	$(MAKE) -C $(patsubst module_%, %, $@)

$(BUILD_DIR):
	mkdir -p $@