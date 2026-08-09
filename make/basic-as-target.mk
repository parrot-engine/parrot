$(BUILD_DIR)/src/%.s.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c -o $@ $<

$(BUILD_DIR)/src/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c -o $@ $<

