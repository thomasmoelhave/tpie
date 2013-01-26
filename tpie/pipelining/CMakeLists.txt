set (PIPE_HEADERS
	buffer.h
	exception.h
	factory_base.h
	factory_helpers.h
	file_stream.h
	graph.h
	helpers.h
	merge.h
	merge_sorter.h
	merger.h
	node.h
	node_map_dump.h
	numeric.h
	pair_factory.h
	pipe_base.h
	pipeline.h
	reverse.h
	sort.h
	std_glue.h
	stdio.h
	tokens.h
	uniq.h
	virtual.h
	)

set (PIPE_SOURCES
	graph.cpp
	pipeline.cpp
	tokens.cpp
	)

add_library(pipelining ${PIPE_HEADERS} ${PIPE_SOURCES})
target_link_libraries(pipelining ${Boost_LIBRARIES})
set_target_properties(pipelining PROPERTIES FOLDER tpie)
