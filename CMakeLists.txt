cmake_minimum_required(VERSION 3.15)
project(parrot)

option(PARROT_DOUBLE_PRECISION "Makes the value of ParrotReal a double instead of a float" OFF)

option(PARROT_VIDEO_MODULE_ENABLE "Builds the video module" ON)


include(ExternalProject)

set(OPTIONS "BUILD_DIR=\"${CMAKE_BINARY_DIR}/parrot_external\"")

set("${OPTIONS} DOUBLE_PRECISION=$<BOOL:${PARROT_DOUBLE_PRECISION}>")

set("${OPTIONS} VIDEO_MODULE_ENABLE=$<BOOL:${PARROT_VIDEO_MODULE_ENABLE}>")

ExternalProject_Add(
    parrot_external
    SOURCE_DIR ${CMAKE_SOURCE_DIR}
    CONFIGURE_COMMAND ""
	BUILD_COMMAND make -f ${CMAKE_CURRENT_SOURCE_DIR}/Makefile ${OPTIONS}
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE 1
	BUILD_ALWAYS 1
)

add_library(parrot STATIC IMPORTED GLOBAL)

set_target_properties(parrot PROPERTIES
    IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/parrot_external/libparrot.a"
	INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_dependencies(parrot parrot_external)
