.PHONY: all

all:
ifneq ($(wildcard $(PLATFORM)),)
	$(MAKE) -C $(PLATFORM) BUILD_DIR=$(BUILD_DIR)/$(PLATFORM)
endif
ifeq ($(PLATFORM_UNIX),1)
	$(MAKE) -C unix BUILD_DIR=$(BUILD_DIR)/unix
endif
