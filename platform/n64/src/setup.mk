ifeq ($(OPTIMIZE),1)
CFLAGS += -Os
endif

ifeq ($(DEBUG_INFO),1)
CFLAGS += -g
endif
