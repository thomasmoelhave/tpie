add_executable(ascii2stream	ascii2stream.cpp)

SET (COMMON_DEPS
	hilbert.cpp
	hilbert.h
	app_config.h
	bulkloader.h
	common.h
	rectangle.h
	rstarnode.h
	rstartree.h
	)
add_library(rtree_common ${COMMON_DEPS})

add_executable(build_rtree build_rtree.cpp)
add_executable(mbr mbr.cpp)
add_executable(testrtree testrtree.cpp)


target_link_libraries(build_rtree tpie rtree_common)
target_link_libraries(testrtree tpie rtree_common)
target_link_libraries(ascii2stream tpie rtree_common)
target_link_libraries(mbr tpie rtree_common)


