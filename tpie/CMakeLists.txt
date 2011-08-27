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
		serialization.h
		sort.h
		stack.h
		stream_arith.h
		stream_compatibility.h
		stream.h
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

set (OTHER_HEADERS
		bit.h   
		bit_matrix.h
		backtrace.h
		b_vector.h
		comparator.h
		config.h.cmake
		cpu_timer.h
		disjoint_sets.h
		execution_time_predictor.h
		internal_sort.h
		internal_queue.h
		internal_stack.h
		internal_vector.h
		internal_stack_vector_base.h
		loglevel.h
		logstream.h
		mergeheap.h
		memory.h
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
		stats_coll.h
		stats.h
		stats_stream.h
		stats_tree.h
		tempname.h
		unused.h
		util.h
		vararray.h
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
		key.cpp
#		matrix_blocks.cpp
	)

set (BTE_SOURCES
	bte/stream_base.cpp
	)

set (OTHER_SOURCES
	#bit.cpp
	#bit_matrix.cpp
	backtrace.cpp
	cpu_timer.cpp
	logstream.cpp
	memory.cpp
	portability.cpp
	tpie_log.cpp
	tempname.cpp
	prime.cpp
	progress_indicator_subindicator.cpp
	progress_indicator_base.cpp
	execution_time_predictor.cpp
	fractional_progress.cpp
	tpie.cpp
	
	)

source_group("BTE" FILES ${BTE_HEADERS} ${BTE_SOURCES})

set(AMI ${AMI_HEADERS} ${AMI_SOURCES})
set(BTE ${BTE_HEADERS} ${BTE_SOURCES})
set(OTHER ${OTHER_HEADERS} ${OTHER_SOURCES})

add_library(tpie ${BTE} ${AMI} ${OTHER})
target_link_libraries(tpie ${Boost_LIBRARIES})

install(TARGETS tpie DESTINATION ${LIB_INSTALL_DIR}) 
install(FILES ${OTHER_HEADERS} ${AMI_HEADERS} DESTINATION ${HEADERS_INSTALL_DIR})
install(FILES ${BTE_HEADERS} DESTINATION ${HEADERS_INSTALL_DIR}/bte)
install(FILES ${IMPORTED_HEADERS} DESTINATION ${HEADERS_INSTALL_DIR}/imported)

