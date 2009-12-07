add_library(kdtree_common app_params.cpp app_params.h)

add_executable(build_btree build_btree.cpp)
add_executable(build_kdbtree build_kdbtree.cpp)
add_executable(build_bkdtree build_bkdtree.cpp)
add_executable(datagen datagen.cpp)

target_link_libraries(build_btree tpie kdtree_common)
target_link_libraries(build_kdbtree tpie kdtree_common)
target_link_libraries(build_bkdtree tpie kdtree_common)
target_link_libraries(datagen tpie kdtree_common)

