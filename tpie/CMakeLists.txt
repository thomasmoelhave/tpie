set (AMI_HEADERS
		block_base.h
		block.h
		btree.h
		cache.h
		cache_base.h
		cache_lru.h
		coll_base.h
		coll.h
		coll_single.h
		err.h
		gen_perm.h
		gen_perm_object.h
		ami.h
		kb_dist.h
		kb_sort.h
		kd_base.h
		kdbtree.h
		kdtree.h
		key.h
		logmethod.h
		merge.h
		merge_sorted_runs.h
		point.h
		priority_queue.h
		priority_queue.inl
		queue.h
		scan.h
		#scan_utils.h
		sort.h
		stack.h
		stream_arith.h
		stream_compatibility.h
		stream.h
		streaming.h
		)

set (BTE_HEADERS
		bte/coll_base.h
		bte/coll.h
		bte/coll_mmap.h
		bte/coll_ufs.h
		bte/err.h
		bte/stack_ufs.h
		bte/stream_base_generic.h
		bte/stream_base.h
		bte/stream_cache.h
		bte/stream.h
		bte/stream_header.h
		bte/stream_mmap.h
		bte/stream_stdio.h
		bte/stream_ufs.h
	)

set (STREAMING_HEADERS
#  streaming/sort.h
#  streaming/buffer.h
#  streaming/stream.h
#  streaming/util.h
#  streaming/memory.h
  )

set (STREAM_HEADERS
#  stream/exception.h
#  stream/concepts.h
#  stream/stdio_bte.h
  )

set (STREAM_SOURCES
#  stream/fd_file_base.cpp
#  stream/stdio_bte.cpp
   file_accessor/stdio.cpp
   file_base.cpp
  )

if (WIN32)
  
else(WIN32)
#  set (STREAM_SOURCES ${STREAM_SOURCES} stream/posix_bte.cpp)
#  set (STREAM_HEADERS ${STREAM_HEADERS} stream/posix_bte.h)
endif(WIN32)

set (OTHER_HEADERS
  concepts.h
  exception.h
		util.h
		bit.h   
		bit_matrix.h
		b_vector.h
		comparator.h
		config.h.cmake
		cpu_timer.h
		internal_sort.h
		logstream.h
		mergeheap.h
		mm_base.h
		mm.h
		mm_manager.h
		persist.h
		portability.h
		pq_internal_heap.h
		pq_internal_heap.inl
		pq_overflow_heap.h
		pq_overflow_heap.inl
		pq_merge_heap.h
		pq_merge_heap.inl
		progress_indicator_arrow.h
		progress_indicator_base.h
		progress_indicator_spin.h
		progress_indicator_terminal.h
		progress_indicator_tpie.h
		sort_manager.h
		stdio_stack.h
		timer.h
		tpie_assert.h
		tpie_log.h
		stats_coll.h
		stats.h
		stats_stream.h
		stats_tree.h
		tempname.h
		vararray.h
	)

set (AMI_SOURCES
	#	bit_permute.cpp
		key.cpp
#		matrix_blocks.cpp
	)

set (BTE_SOURCES
	bte/stream_base.cpp
	)

set (OTHER_SOURCES
#        streaming/memory.cpp
	#bit.cpp
	#bit_matrix.cpp
	cpu_timer.cpp
	logstream.cpp
	mm_base.cpp
	mm_manager.cpp
	portability.cpp
	tpie_log.cpp
	tempname.cpp
	util.cpp
	)

source_group("BTE" FILES ${BTE_HEADERS} ${BTE_SOURCES})

set(AMI ${AMI_HEADERS} ${AMI_SOURCES})
set(BTE ${BTE_HEADERS} ${BTE_SOURCES})
set(OTHER ${OTHER_HEADERS} ${OTHER_SOURCES})
set(STREAM ${STREAM_HEADERS} ${STREAM_SOURCES})
set(STREAMING ${STREAMING_HEADERS})

add_library(tpie ${BTE} ${AMI} ${OTHER} ${STREAMING} ${STREAM})
target_link_libraries(tpie ${Boost_LIBRARIES})

install(TARGETS tpie DESTINATION ${LIB_INSTALL_DIR}) 
install(FILES ${OTHER_HEADERS} ${AMI_HEADERS} DESTINATION ${HEADERS_INSTALL_DIR})
install(FILES ${BTE_HEADERS} DESTINATION ${HEADERS_INSTALL_DIR}/bte)
install(FILES ${STREAMING_HEADERS} DESTINATION ${HEADERS_INSTALL_DIR}/streaming)

