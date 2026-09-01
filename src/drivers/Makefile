SRCS=# $(shell echo *.c)
OBJS=$(patsubst %, $(BUILD_DIR)/src/%.o, $(SRCS))

.PHONY: all
all: $(OBJS)
	$(foreach DRIVER, $(DRIVERS), $(MAKE) -C $(DRIVER) BUILD_DIR=$(BUILD_DIR)/$(DRIVER);)

include $(ROOT_DIR)/make/basic-targets.mk
include $(ROOT_DIR)/make/cc-deps.mk
