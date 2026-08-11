#######################################################################################################################
# DEFAULT CONFIGURATION
# PLEASE DO NOT EDIT FOR YOUR PERSONAL CONFIGURATION. PEFER config/ FOR THIS TASK
#######################################################################################################################

# [see the contents of platform/ for the avaliable values]
# The platform to build on. By default, set to the host OS if the makefile can detect it, otherwise, targets Linux.
export PLATFORM ?= $(DEFAULT)
# [string]
# The architecture. This can be forced to a value by the platform if it is single-architecture. By default, set to the
# host architecture if the makefile can detect it, otherwise, it targets x86_64.
export ARCH ?= $(DEFAULT)

# [bool] Build the code in debug mode. Affects defaults of several configuration options and if enabled,
# adds several debugging features.
export DEBUG ?= 0

# [bool] Generate compile_commands.json
export COMPILE_DB ?= 0

# [bool] Build the code with optimizations. The default value is the opposite of if debug mode is enabled.
export OPTIMIZE ?= $(DEFAULT)
# [bool] Build with debug info for debuggers and other tools. The default value matches if debug mode is enabled.
export DEBUG_INFO ?= $(DEFAULT)

# [bool] Makes the value of ParrotReal a double instead of a float
export DOUBLE_PRECISION ?= 0

# [bool] Avoids using memory as much as possible. Use on systems with low amounts of memory or when it's the bottleneck.
# Default is platform dependent but if the platform specifies no default, it will be treated as false.
export AVOID_MEMORY ?= $(DEFAULT)

# [bool] If to build the video module
export VIDEO_MODULE_ENABLE ?= 1

# [path] The build directory name (relative to the root build directory) to build into. By default, the value is derived from the
# other options.
export BUILD_DIR_NAME = $(DEFAULT)


#######################################################################################################################

_UNAME=$(shell uname)

ifeq ($(_UNAME),Linux)
ifeq ($(PLATFORM),$(DEFAULT))
export PLATFORM = linux
endif
endif

ifeq ($(ARCH),$(DEFAULT))
export ARCH = $(shell uname -m)
endif

ifeq ($(PLATFORM),$(DEFAULT))
export PLATFORM = linux
endif

ifeq ($(ARCH),$(DEFAULT))
export ARCH = x86_64
endif

-include $(ROOT_DIR)/platform/$(PLATFORM)/defaults.mk

ifeq ($(OPTIMIZE),$(DEFAULT))
ifeq ($(DEBUG),1)
export OPTIMIZE = 0
endif
ifeq ($(DEBUG),0)
export OPTIMIZE = 1
endif
endif

ifeq ($(DEBUG_INFO),$(DEFAULT))
export DEBUG_INFO = $(DEBUG)
endif

ifeq ($(AVOID_MEMORY),$(DEFAULT))
export AVOID_MEMORY = 0
endif

ifeq ($(BUILD_DIR_NAME),$(DEFAULT))
export BUILD_DIR_NAME=$(PLATFORM)
ifneq ($(ARCH),)
export BUILD_DIR_NAME := $(BUILD_DIR_NAME)-$(ARCH)
endif
ifeq ($(DEBUG),1)
export BUILD_DIR_NAME := $(BUILD_DIR_NAME)+debug
endif
ifeq ($(OPTIMIZE),1)
export BUILD_DIR_NAME := $(BUILD_DIR_NAME)+optimized
endif
ifeq ($(DEBUG_INFO),1)
export BUILD_DIR_NAME := $(BUILD_DIR_NAME)+dbginfo
endif

ifeq ($(VIDEO_MODULE_ENABLE),1)
export BUILD_DIR_NAME := $(BUILD_DIR_NAME)+video
endif
endif
