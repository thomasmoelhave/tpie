set (PIPE_HEADERS
	buffer.h
	core.h
	exception.h
	factory_helpers.h
	file_stream.h
	graph.h
	helpers.h
	merge.h
	merge_sorter.h
	merger.h
	numeric.h
	pipe_segment.h
	reverse.h
	segment_map_dump.h
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
