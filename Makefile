export DEFAULT=__default__

export ROOT_DIR=$(shell pwd)

export MAKE=make

CONFIG_FILE=

ifneq ($(CONFIG_FILE),)
include $(CONFIG_FILE)
endif

include config.default.mk

PREFIX ?= /usr/local

export ROOT_BUILD_DIR=$(ROOT_DIR)/build
export BUILD_DIR=$(ROOT_BUILD_DIR)/$(BUILD_DIR_NAME)

export INCLUDE_DIR=$(ROOT_DIR)/include
export BUILD_INCLUDE_DIR=$(BUILD_DIR)/include

export CFLAGS += $(EXTRA_CFLAGS) $(EXTRA_CLDFLAGS)
export ASFLAGS += $(EXTRA_ASFLAGS)
export LDFLAGS += $(EXTRA_LDFLAGS) $(EXTRA_CLDFLAGS)
export CFLAGS += -I$(INCLUDE_DIR) -I$(BUILD_INCLUDE_DIR) -I$(ROOT_DIR)/platform/$(PLATFORM)/include

export OUTPUT_NAME=libparrot
export LIB_OUTPUT_STATIC=$(BUILD_DIR)/$(OUTPUT_NAME).a

ifeq ($(COMPILE_DB),1)
BEAR=bear -a -- 
endif

ifeq ($(COMPILE_DB),0)
BEAR=
endif

include platform/$(PLATFORM)/setup.mk

.PHONY: all
.PHONY: src
.PHONY: compile-db test build-test
.PHONY: install uninstall
.PHONY: clean clean-all

all: $(LIB_OUTPUT_STATIC)
	echo "$(CFLAGS)" > $(BUILD_DIR)/$(OUTPUT_NAME).cflags 
	echo "$(LDFLAGS)" > $(BUILD_DIR)/$(OUTPUT_NAME).ldflags 
	echo "$(LDLIBS)" > $(BUILD_DIR)/$(OUTPUT_NAME).ldlibs

$(LIB_OUTPUT_STATIC): src

src: | $(BUILD_DIR)
	$(BEAR) $(MAKE) -C src BUILD_DIR=$(BUILD_DIR)/lib "DRIVERS=$(DRIVERS)"

test: build-test
	$(MAKE) -C test BUILD_DIR=$(BUILD_DIR)/test run

build-test: $(LIB_OUTPUT_STATIC)
	$(BEAR) $(MAKE) -C test BUILD_DIR=$(BUILD_DIR)/test

install: $(LIB_OUTPUT_STATIC) $(BUILD_DIR)/parrot.pc
	install -d $(DESTDIR)$(PREFIX)/lib
	
	install -m 644 $(LIB_OUTPUT_STATIC) $(DESTDIR)$(PREFIX)/lib/$(OUTPUT_NAME).a
	
	install -d $(DESTDIR)$(PREFIX)/include/parrot
	cp -r $(INCLUDE_DIR)/parrot/* $(BUILD_INCLUDE_DIR)/parrot/* $(DESTDIR)$(PREFIX)/include/parrot
	chmod -R 644 $(DESTDIR)$(PREFIX)/include/parrot
	find $(DESTDIR)$(PREFIX)/include/parrot -type d -exec chmod 755 {} +
	
	install -d $(DESTDIR)$(PREFIX)/lib/pkgconfig
	install -m 644 $(BUILD_DIR)/parrot.pc $(DESTDIR)$(PREFIX)/lib/pkgconfig/parrot.pc

uninstall:
	rm $(DESTDIR)$(PREFIX)/lib/$(OUTPUT_NAME).a
	
	rm -r $(DESTDIR)$(PREFIX)/include/parrot
	
	rm $(DESTDIR)$(PREFIX)/lib/pkgconfig/parrot.pc

$(BUILD_DIR)/parrot.pc: parrot.pc.in | $(BUILD_DIR)
	sed -e "s|@PREFIX@|$(PREFIX)|g" \
		-e "s|@LDLIBS@|$(LDLIBS)|g" \
		$< > $@

clean:
	rm -r $(BUILD_DIR)

clean-all:
	rm -r $(ROOT_BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
