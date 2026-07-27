ifeq ($(OPTIMIZE),1)
export CFLAGS += -O2
endif

ifeq ($(DEBUG_INFO),1)
export CFLAGS += -g
endif
