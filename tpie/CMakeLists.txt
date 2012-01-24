set_property(GLOBAL PROPERTY USE_FOLDERS On)

set (AMI_HEADERS
		err.h
		ami.h
		merge.h
		merge_sorted_runs.h
		priority_queue.h
		priority_queue.inl
		queue.h
		serialization.h
		sort.h
		stack.h
		stream_compatibility.h
		stream.h
		)

set (STREAM_HEADERS
#	stream/exception.h
#	stream/concepts.h
#	stream/stdio_bte.h
	)

set (STREAM_SOURCES
#	stream/fd_file_base.cpp
#	stream/stdio_bte.cpp
	 file_accessor/file_accessor.cpp
	 file_accessor/stdio.cpp
	 file_base.cpp
	)

if (WIN32)
set (STREAM_SOURCES ${STREAM_SOURCES} file_accessor/win32.cpp)
set (STREAM_HEADERS ${STREAM_HEADERS} file_accessor/win32.h)
else(WIN32)
set (STREAM_SOURCES ${STREAM_SOURCES} file_accessor/posix.cpp)
set (STREAM_HEADERS ${STREAM_HEADERS} file_accessor/posix.h)
endif(WIN32)

set (OTHER_HEADERS
		backtrace.h
		comparator.h
		config.h.cmake
		cpu_timer.h
		disjoint_sets.h
		exception.h
		file_count.h
		execution_time_predictor.h
		internal_sort.h
		internal_queue.h
		internal_stack.h
		internal_vector.h
		internal_stack_vector_base.h
		job.h
		loglevel.h
		logstream.h
		mergeheap.h
		memory.h
		memory.inl
		persist.h
		portability.h
		internal_priority_queue.h
		pq_overflow_heap.h
		pq_overflow_heap.inl
		pq_merge_heap.h
		pq_merge_heap.inl
		fractional_progress.h
		parallel_sort.h
		dummy_progress.h
		progress_indicator_subindicator.h
		progress_indicator_arrow.h
		progress_indicator_base.h
		progress_indicator_spin.h
		progress_indicator_null.h
		progress_indicator_terminal.h
		progress_indicator_tpie.h
		sort_manager.h
		stdio_stack.h
		timer.h
		tpie_assert.h
		tpie_log.h
		stats.h
		stats_stream.h
		stats_tree.h
		tempname.h
		unused.h
		util.h
		array.h
		bit_array.h
		packed_array.h
		hash_map.h
		prime.h
		concepts.h
		concept_doc.h
		tpie.h
		)

set (IMPORTED_HEADERS
	imported/cycle.h
)

set (AMI_SOURCES
	#	bit_permute.cpp
#		matrix_blocks.cpp
	)

set (OTHER_SOURCES
	backtrace.cpp
	cpu_timer.cpp
	file_count.cpp
	execution_time_predictor.cpp
	fractional_progress.cpp
	job.cpp
	logstream.cpp
	memory.cpp
	portability.cpp
	prime.cpp
	progress_indicator_base.cpp
	progress_indicator_subindicator.cpp
	tempname.cpp
	tpie.cpp
	tpie_log.cpp
	util.cpp
	)

set(AMI ${AMI_HEADERS} ${AMI_SOURCES})
set(OTHER ${OTHER_HEADERS} ${OTHER_SOURCES})
set(STREAM ${STREAM_HEADERS} ${STREAM_SOURCES})

add_library(tpie ${AMI} ${OTHER} ${STREAM})
target_link_libraries(tpie ${Boost_LIBRARIES})
set_target_properties(tpie PROPERTIES FOLDER tpie)

install(TARGETS tpie DESTINATION ${LIB_INSTALL_DIR}) 
install(FILES ${OTHER_HEADERS} ${AMI_HEADERS} DESTINATION ${HEADERS_INSTALL_DIR})
install(FILES ${IMPORTED_HEADERS} DESTINATION ${HEADERS_INSTALL_DIR}/imported)

