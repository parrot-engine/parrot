TARGETS=lib

ifeq ($(N64_PROVIDE_OS),1)
TARGETS += os
endif

.PHONY: lib os

all: $(TARGETS)

lib:
	$(MAKE) -C lib BUILD_DIR=$(BUILD_DIR)/lib

os:
	$(MAKE) -C os BUILD_DIR=$(BUILD_DIR)/os
