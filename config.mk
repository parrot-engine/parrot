#######################################################################################################################
# DEFAULT CONFIGURATION
# PLEASE DO NOT EDIT FOR YOUR PERSONAL CONFIGURATION. PEFER config/ FOR THIS TASK
#######################################################################################################################

# The platform to build on. By default, set to the host OS if the makefile can detect it, otherwise, targets Linux.
export CONFIG_PLATFORM ?= $(DEFAULT)
# [string]
# The architecture. This can be forced to a value by the platform if it is single-architecture. By default, set to the
# host architecture if the makefile can detect it, otherwise, it targets x86_64.
export CONFIG_ARCH ?= $(DEFAULT)

# [bool] Build the code in debug mode. Affects defaults of several configuration options and if enabled,
# adds several debugging features.
export CONFIG_DEBUG ?= 0

# [bool] Generate compile_commands.json
export CONFIG_COMPILE_DB ?= 0

# [bool] Build the code with optimizations. The default value is the opposite of if debug mode is enabled.
export CONFIG_OPTIMIZE ?= $(DEFAULT)
# [bool] Build with debug info for debuggers and other tools. The default value matches if debug mode is enabled.
export CONFIG_DEBUG_INFO ?= $(DEFAULT)
# [bool] Build with strict compiler flags. The default value matches if debug mode is enabled.
export CONFIG_STRICT ?= $(DEFAULT)

# [bool] Makes the value of ParrotReal a double instead of a float
export CONFIG_DOUBLE_PRECISION ?= 0

# [bool] Makes the engine provide a libc
export CONFIG_PROVIDE_LIBC ?= $(DEFAULT)

# [path] The build directory name (relative to the root build directory) to build into. By default, the value is derived from the
# other options.
export CONFIG_BUILD_DIR_NAME ?= $(DEFAULT)

# [set: default, x11] List of drivers to build. `default` includes platform default drivers
export CONFIG_DRIVERS ?= default

# [string] Extra CFLAGS to add to the build
export CONFIG_EXTRA_CFLAGS ?= 

# [string] Extra ASFLAGS to add to the build
export CONFIG_EXTRA_ASFLAGS ?= 

# [string] Extra LDFLAGS to add to the build
export CONFIG_EXTRA_LDFLAGS ?= 

# [string] Extra CFLAGS and LDFLAGS to add to the build
export CONFIG_EXTRA_CLDFLAGS ?= 

_UNAME=$(shell uname)

ifeq ($(_UNAME),Linux)
ifeq ($(CONFIG_PLATFORM),$(DEFAULT))
export CONFIG_PLATFORM = linux
endif
endif

ifeq ($(CONFIG_ARCH),$(DEFAULT))
export CONFIG_ARCH = $(shell uname -m)
endif

ifeq ($(CONFIG_PLATFORM),$(DEFAULT))
export CONFIG_PLATFORM = linux
endif

ifeq ($(CONFIG_ARCH),$(DEFAULT))
export CONFIG_ARCH = x86_64
endif

ifeq ($(CONFIG_OPTIMIZE),$(DEFAULT))
ifeq ($(CONFIG_DEBUG),1)
export CONFIG_OPTIMIZE = 0
endif
ifeq ($(CONFIG_DEBUG),0)
export CONFIG_OPTIMIZE = 1
endif
endif

ifeq ($(CONFIG_DEBUG_INFO),$(DEFAULT))
export CONFIG_DEBUG_INFO = $(DEBUG)
endif

ifeq ($(CONFIG_STRICT),$(DEFAULT))
export CONFIG_STRICT = $(DEBUG)
endif
