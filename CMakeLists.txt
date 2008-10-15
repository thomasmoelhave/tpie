cmake_minimum_required(VERSION 2.6)

project(tpie)

include_directories(tpie)
add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)


#### CONFIG.H Checks:
include(CheckIncludeFiles)

check_include_files("unistd.h" TPIE_HAVE_UNISTD_H)
check_include_files("sys/unistd.h" TPIE_HAVE_SYS_UNISTD_H)
check_include_files("sys/async.h" TPIE_HAVE_LIBAIO)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tpie/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


add_subdirectory(tpie)

option(COMPILE_TEST "Compile test programs" OFF)

if (COMPILE_TEST)
	add_subdirectory(test)
endif (COMPILE_TEST)
