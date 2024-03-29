set_property(GLOBAL PROPERTY USE_FOLDERS On)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/sysinfo.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/sysinfo.cpp" @ONLY)

set (HEADERS
		access_type.h
		backtrace.h
		blocks/block.h
		blocks/block_collection.h
		blocks/block_collection_cache.h
		blocks/freespace_collection.h
		btree.h
		btree/base.h
		btree/internal_store.h
		btree/external_store.h
		btree/external_store_base.h
		btree/serialized_store.h
		btree/node.h
		btree/btree.h
		btree/btree_builder.h
		cache_hint.h
		comparator.h
		compressed/buffer.h
		compressed/direction.h
		compressed/predeclare.h
		compressed/request.h
		compressed/scheme.h
		compressed/stream.h
		compressed/stream_position.h
		compressed/thread.h
		config.h.cmake
		cpu_timer.h
		deprecated.h
		disjoint_sets.h
		exception.h
		err.h
		file.h
		file_base.h
		file_base_crtp.h
		file_manager.h
		file_stream.h
		file_stream_base.h
		file_base_crtp.inl
		file_accessor/byte_stream_accessor.h
		file_accessor/file_accessor.h
		file_accessor/stream_accessor.h
		file_accessor/stream_accessor_base.h
		file_accessor/stream_accessor_base.inl
		flags.h
		execution_time_predictor.h
		internal_sort.h
		internal_queue.h
		internal_stack.h
		internal_vector.h
		internal_stack_vector_base.h
		is_simple_iterator.h
		job.h
		loglevel.h
		logstream.h
		mergeheap.h
		merge_sorted_runs.h
		memory.h
		persist.h
		pipelining.h
		pipelining/ami_glue.h
		pipelining/buffer.h
		pipelining/chunker.h
		pipelining/container.h
		pipelining/exception.h
		pipelining/factory_base.h
		pipelining/factory_helpers.h
		pipelining/file_stream.h
		pipelining/filter.h
		pipelining/forwarder.h
		pipelining/helpers.h
		pipelining/internal_buffer.h
		pipelining/join.h
		pipelining/maintain_order_type.h
		pipelining/map.h
		pipelining/merge.h
		pipelining/merge_sorter.h
		pipelining/merger.h
		pipelining/node.h
		pipelining/node_map_dump.h
		pipelining/node_name.h
		pipelining/node_set.h
		pipelining/node_traits.h
		pipelining/numeric.h
		pipelining/ordered_merge.h
		pipelining/pair_factory.h
		pipelining/parallel.h
		pipelining/parallel/aligned_array.h
		pipelining/parallel/base.h
		pipelining/parallel/factory.h
		pipelining/parallel/options.h
		pipelining/parallel/pipes.h
		pipelining/parallel/worker_state.h
		pipelining/pipe_base.h
		pipelining/pipeline.h
		pipelining/predeclare.h
		pipelining/priority_type.h
		pipelining/reverse.h
		pipelining/serialization.h
		pipelining/serialization_sort.h
		pipelining/sort.h
		pipelining/sort_parameters.h
		pipelining/split.h
		pipelining/std_glue.h
		pipelining/stdio.h
		pipelining/store.h
		pipelining/subpipeline.h
		pipelining/tokens.h
		pipelining/uniq.h
		pipelining/virtual.h
		pipelining/visit.h
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
		resource_manager.h
		resources.h
		serialization2.h
		serialization_stream.h
		serialization_sorter.h
		sort.h
		sort_deprecated.h
		sort_manager.h
		stack.h
		stream.h
		stream_crtp.h
		stream_crtp.inl
		stream_header.h
		stream_old.h
		stream_usage.h
		stream_writable.h
		sysinfo.h
		tpie_assert.h
		tpie_log.h
		tuple_utils.h
		stats.h
		types.h
		tempname.h
		uncompressed_stream.h
		unused.h
		util.h
		array.h
		bit_array.h
		packed_array.h
		array_view_base.h
		array_view.h
		concept_doc.h
		tpie.h
		unittest.h
		maybe.h
		tiny.h
		)

set (SOURCES
	backtrace.cpp
	blocks/block_collection.cpp
	blocks/block_collection_cache.cpp
	btree/external_store_base.cpp
	compressed/buffer.cpp
	compressed/request.cpp
	compressed/scheme_none.cpp
	compressed/scheme_snappy.cpp
	compressed/stream_base.cpp
	compressed/thread.cpp
	cpu_timer.cpp
	file_base.cpp
	file_manager.cpp
	file_stream_base.cpp
	execution_time_predictor.cpp
	fractional_progress.cpp
	job.cpp
	logstream.cpp
	memory.cpp
	pipelining/merge_sorter.cpp
	pipelining/node.cpp
	pipelining/node_name.cpp
	pipelining/pipeline.cpp
	pipelining/runtime.cpp
	pipelining/tokens.cpp
	pipelining/factory_base.cpp
	portability.cpp
	progress_indicator_base.cpp
	progress_indicator_subindicator.cpp
	resource_manager.cpp
	resources.cpp
	serialization_stream.cpp
	tempname.cpp
	tpie.cpp
	tpie_log.cpp
	stats.cpp
	util.cpp
	unittest.cpp
	sysinfo2.cpp
	"${CMAKE_CURRENT_BINARY_DIR}/sysinfo.cpp"
	)

if (WIN32)
	set(HEADERS ${HEADERS} file_accessor/win32.h file_accessor/win32.inl)
else()
	set(HEADERS ${HEADERS} file_accessor/posix.h file_accessor/posix.inl)
endif()

if (TPIE_SHARED)
  add_library(tpie SHARED ${HEADERS} ${SOURCES})
else()
  add_library(tpie STATIC ${HEADERS} ${SOURCES})
endif()

#TODO this is quite bad we inject everything into the include path
target_include_directories(tpie PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/..)

set_target_properties(tpie PROPERTIES
  LINKER_LANGUAGE CXX
  C_STANDARD 17
  C_STANDARD_REQUIRED ON
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_VISIBILITY_PRESET hidden

  FOLDER tpie

  VERSION ${GIT_COMMIT}
  POSITION_INDEPENDENT_CODE ON
)
target_compile_features(tpie PUBLIC cxx_std_17)

target_link_libraries(tpie PUBLIC Boost::boost Threads::Threads)
target_compile_definitions(tpie PUBLIC -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)

include(GenerateExportHeader)
generate_export_header(tpie
  DEFINE_NO_DEPRECATED
)

if(NOT WIN32)
	target_compile_options(tpie PUBLIC -Wall -Wextra)
endif()

if(WIN32)
	target_compile_options(tpie PUBLIC -DWIN32_LEAN_AND_MEAN -DNOGDI /bigobj)
	target_link_libraries(tpie PRIVATE wsock32 ws2_32)
endif()

if(TPIE_HAS_SNAPPY)
	target_link_libraries(tpie PUBLIC Snappy::Snappy)
endif()

if(TPIE_HAS_LZ4)
	target_link_libraries(tpie PUBLIC lz4::lz4)
endif()

if(TPIE_HAS_ZSTD)
	target_link_libraries(tpie PUBLIC zstd::zstd)
endif()

install(TARGETS tpie
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)
