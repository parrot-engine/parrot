$(BUILD_DIR)/src/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
ifeq ($(DEBUG_INFO),0)
	$(STRIP) $@
endif