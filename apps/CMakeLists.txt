add_subdirectory(matrix)
add_subdirectory(rtree)
add_subdirectory(bkdtree)

add_executable(testtpie tpie.cpp)
target_link_libraries(testtpie tpie)
