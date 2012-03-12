set_property(GLOBAL PROPERTY USE_FOLDERS On)

set (HEADERS
		ami.h
		backtrace.h
		comparator.h
		config.h.cmake
		cpu_timer.h
		deprecated.h
		disjoint_sets.h
		exception.h
		err.h
		file.h
		file_stream.h
		file_accessor/file_accessor_crtp.h
		file_accessor/file_accessor_crtp.inl
		file_accessor/file_accessor.h
		file_count.h
		execution_time_predictor.h
		imported/cycle.h
		internal_sort.h
		internal_queue.h
		internal_stack.h
		internal_vector.h
		internal_stack_vector_base.h
		job.h
		loglevel.h
		logstream.h
		merge.h
		mergeheap.h
		merge_sorted_runs.h
		memory.h
		memory.inl
		persist.h
		portability.h
		internal_priority_queue.h
		priority_queue.inl
		priority_queue.h
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
		queue.h
		serialization.h
		sort.h
		sort_manager.h
		stack.h
		stream.h
		stream_header.h
		stream_usage.h
		tpie_assert.h
		tpie_log.h
		types.h
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

set (SOURCES
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
	file_accessor/file_accessor.cpp
	file_accessor/stdio.cpp
	file_base.cpp
	)

if (WIN32)
set (SOURCES ${SOURCES} file_accessor/win32.cpp)
set (HEADERS ${HEADERS} file_accessor/win32.h)
else(WIN32)
set (SOURCES ${SOURCES} file_accessor/posix.cpp)
set (HEADERS ${HEADERS} file_accessor/posix.h)
endif(WIN32)

add_library(tpie ${HEADERS} ${SOURCES})
target_link_libraries(tpie ${Boost_LIBRARIES})
set_target_properties(tpie PROPERTIES FOLDER tpie)

install(TARGETS tpie DESTINATION ${LIB_INSTALL_DIR}) 
install(FILES ${HEADERS} DESTINATION ${HEADERS_INSTALL_DIR})
