all:
	$(MAKE) -C n64 BUILD_DIR=$(BUILD_DIR)/n64

ifeq ($(VIDEO_MODULE_ENABLE),1)
	$(MAKE) -C video BUILD_DIR=$(BUILD_DIR)/video
endif
