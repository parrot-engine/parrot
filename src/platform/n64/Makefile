.PHONY: all

all:
ifeq ($(N64_PROVIDE_OS),1)
	$(MAKE) -C os BUILD_DIR=$(BUILD_DIR)/os
endif
	$(MAKE) -C lib BUILD_DIR=$(BUILD_DIR)/lib
