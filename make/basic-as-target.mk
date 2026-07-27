$(BUILD_DIR)/src/%.s.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c -o $@ $<
