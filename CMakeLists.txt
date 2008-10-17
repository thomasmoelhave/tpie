cmake_minimum_required(VERSION 2.6)

project(tpie)

include_directories(.)
add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

#### CONFIG.H Checks:
include(CheckIncludeFiles)

if(NOT WIN32) 
	add_definitions("-Wall")
endif(NOT WIN32)

check_include_files("unistd.h" TPIE_HAVE_UNISTD_H)
check_include_files("sys/unistd.h" TPIE_HAVE_SYS_UNISTD_H)
check_include_files("sys/async.h" TPIE_HAVE_LIBAIO)


include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tpie/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/tpie/config.h)
add_subdirectory(tpie)


add_subdirectory(doc)

option(COMPILE_TEST "Compile test programs" ON)

if (COMPILE_TEST)
	add_subdirectory(test)
endif (COMPILE_TEST)

add_subdirectory(apps)

