cmake_minimum_required(VERSION 2.6)

project(tpie)

include_directories(include)
add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)

add_subdirectory(lib)
