export PLATFORM_HAS_LIBC ?= 1
export PLATFORM_UNIX ?= 1

override DRIVERS := $(subst default,x11 gl11,$(DRIVERS))
