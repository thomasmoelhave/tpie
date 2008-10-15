set (AMI_HEADERS
		block_base.h
		block.h
		btree.h
		cache.h
		coll_base.h
		coll.h
		coll_single.h
		device.h
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
		matrix_blocks.h
		matrix_fill.h
		matrix.h
		matrix_pad.h
		merge.h
		merge_sorted_runs.h
		point.h
		queue.h
		scan.h
		#scan.h.head
		#scan.h.tail
		#scan_mac.cpp
		#scan_mac.h
		#scan_utils.h
		sort.h
		sparse_matrix.h
		stack.h
		stream_arith.h
		stream_base.h
		stream_compatibility.h
		stream.h
		stream_single.h
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
		b_vector.h
		comparator.h
		config.h
		config.h.cmake
		cpu_timer.h
		internal_sort.h
		logstream.h
		matrix.h
		mergeheap.h
		mm_base.h
		mm.h
		mm_register.h
		persist.h
		portability.h
		progress_indicator_arrow.h
		progress_indicator_base.h
		progress_indicator_spin.h
		progress_indicator_terminal.h
		progress_indicator_tpie.h
		quicksort.h
		sort_manager.h
		stdio_stack.h
		timer.h
		tpie_assert.h
		tpie_log.h
		tpie_stats_coll.h
		tpie_stats.h
		tpie_stats_stream.h
		tpie_stats_tree.h
		tempname.h
		vararray.h
		versions.h
		lib_config.h
		)

set (AMI_SOURCES
	#	bit_permute.cpp
		device.cpp
		key.cpp
#		matrix_blocks.cpp
		stream_base.cpp
	)

set (BTE_SOURCES
	bte/stream_base.cpp
	)

set (OTHER_SOURCES
	#bit.cpp
	#bit_matrix.cpp
	cpu_timer.cpp
	logstream.cpp
	mm_base.cpp
	mm_register.cpp
	portability.cpp
	log.cpp
	tempname.cpp
	)

source_group("BTE" FILES ${BTE_HEADERS} ${BTE_SOURCES})

set(AMI ${AMI_HEADERS} ${AMI_SOURCES})
set(BTE ${BTE_HEADERS} ${BTE_SOURCES})
set(OTHER ${OTHER_HEADERS} ${OTHER_SOURCES})

add_library(tpie ${BTE} ${AMI} ${OTHER})

install(TARGETS tpie DESTINATION lib)
