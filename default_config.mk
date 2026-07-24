export PLATFORM ?= linux_x86_64

export DEBUG ?= 0

ifeq ($(DEBUG),1)
export DEBUG_INFO ?= 1
export OPTIMIZE ?= 0
endif

ifeq ($(DEBUG),0)
export DEBUG_INFO ?= 0
export OPTIMIZE ?= 1
endif

export BUILD_STATIC ?= 1
export BUILD_SHARED ?= 1

export REAL_NUMBER_TYPE ?= float

export FORCE_EXPECT_SLOW_MEMORY ?= 0

export MODULE_SCENE_ENABLE ?= 1

export MODULE_VIDEO_ENABLE ?= 1