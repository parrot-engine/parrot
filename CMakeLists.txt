cmake_minimum_required(VERSION 3.15)
project(parrot)

option(PARROT_DOUBLE_PRECISION "Makes the value of ParrotReal a double instead of a float" OFF)

set(PARROT_DRIVERS "default" CACHE STRING "The drivers to include in the build")


include(ExternalProject)

set(OPTIONS
    BUILD_DIR=${CMAKE_BINARY_DIR}/parrot_external
    DOUBLE_PRECISION=$<BOOL:${PARROT_DOUBLE_PRECISION}>
    DRIVERS=${PARROT_DRIVERS}
)

ExternalProject_Add(
    parrot_external
    SOURCE_DIR ${CMAKE_SOURCE_DIR}
    CONFIGURE_COMMAND ""
	BUILD_COMMAND make -C ${CMAKE_CURRENT_SOURCE_DIR} ${OPTIONS}
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE 1
	BUILD_ALWAYS 1
)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/parrot_external/include")

add_library(parrot STATIC IMPORTED GLOBAL)

set_target_properties(parrot PROPERTIES
    IMPORTED_LOCATION "${CMAKE_BINARY_DIR}/parrot_external/libparrot.a"
	INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include;${CMAKE_BINARY_DIR}/parrot_external/include"
)

add_dependencies(parrot parrot_external)
