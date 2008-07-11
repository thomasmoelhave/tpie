cmake_minimum_required(VERSION 2.6)

project(tpie)

include_directories(include)
add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)


#### CONFIG.H Checks:
include(CheckIncludeFiles)

check_include_files("unistd.h" HAVE_UNISTD_H)
check_include_files("sys/unistd.h" HAVE_SYS_UNISTD_H)
check_include_files("sys/async.h" HAVE_LIBAIO)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


add_subdirectory(lib)
