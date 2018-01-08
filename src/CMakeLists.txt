set_property(GLOBAL PROPERTY USE_FOLDERS On)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/sysinfo.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/sysinfo.cpp" @ONLY)

add_library(tpie
  backtrace.cpp
  blocks/block_collection.cpp
  blocks/block_collection_cache.cpp
  btree/external_store_base.cpp
  btree/serialized_store.cpp
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
  portability.cpp
  prime.cpp
  progress_indicator_base.cpp
  progress_indicator_subindicator.cpp
  resource_manager.cpp
  resources.cpp
  serialization_stream.cpp
  hash.cpp
  jsonprint.cpp
  tempname.cpp
  tpie.cpp
  tpie_log.cpp
  stats.cpp
  util.cpp
  unittest.cpp
  "${CMAKE_CURRENT_BINARY_DIR}/sysinfo.cpp"
  )
target_link_libraries(tpie PUBLIC Boost::boost Boost::filesystem Boost::system Boost::date_time Threads::Threads)
set_target_properties(tpie PROPERTIES FOLDER tpie)
target_include_directories(tpie PUBLIC ../include ${CMAKE_CURRENT_BINARY_DIR}/../include PRIVATE .)

if(TPIE_USE_SNAPPY)
	target_link_libraries(tpie PRIVATE snappy)
endif(TPIE_USE_SNAPPY)

if(TPIE_USE_LZ4)
	target_link_libraries(tpie PRIVATE lz4)
endif(TPIE_USE_LZ4)

if(TPIE_USE_ZSTD)
	target_link_libraries(tpie PRIVATE zstd)
endif(TPIE_USE_ZSTD)

install(TARGETS tpie
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib)
