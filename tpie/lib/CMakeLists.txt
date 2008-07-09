set (AMI_HEADERS
		../include/ami_block_base.h
		../include/ami_block.h
		../include/ami_btree.h
		../include/ami_cache.h
		../include/ami_coll_base.h
		../include/ami_coll.h
		../include/ami_coll_single.h
		../include/ami_device.h
		../include/ami_err.h
		../include/ami_gen_perm.h
		../include/ami_gen_perm_object.h
		../include/ami.h
		../include/ami_kb_dist.h
		../include/ami_kb_sort.h
		../include/ami_kd_base.h
		../include/ami_kdbtree.h
		../include/ami_kdtree.h
		../include/ami_key.h
		../include/ami_logmethod.h
		../include/ami_matrix_blocks.h
		../include/ami_matrix_fill.h
		../include/ami_matrix.h
		../include/ami_matrix_pad.h
		../include/ami_merge.h
		../include/ami_merge_sorted_runs.h
		../include/ami_point.h
		../include/ami_queue.h
		../include/ami_scan.h
		#../include/ami_scan.h.head
		#../include/ami_scan.h.tail
		#../include/ami_scan_mac.cpp
		#../include/ami_scan_mac.h
		#../include/ami_scan_utils.h
		../include/ami_sort.h
		../include/ami_sparse_matrix.h
		../include/ami_stack.h
		../include/ami_stream_arith.h
		../include/ami_stream_base.h
		../include/ami_stream_compatibility.h
		../include/ami_stream.h
		../include/ami_stream_single.h
		)

set (BTE_HEADERS
		../include/bte_coll_base.h
		../include/bte_coll.h
		../include/bte_coll_mmap.h
		../include/bte_coll_ufs.h
		../include/bte_err.h
		../include/bte_stack_ufs.h
		../include/bte_stream_base_generic.h
		../include/bte_stream_base.h
		../include/bte_stream_cache.h
		../include/bte_stream.h
		../include/bte_stream_header.h
		../include/bte_stream_mmap.h
		../include/bte_stream_stdio.h
		../include/bte_stream_ufs.h
	)

set (OTHER_HEADERS
		../include/bit.h   
		../include/bit_matrix.h
		../include/b_vector.h
		../include/comparator.h
		../include/config.h
		../include/cpu_timer.h
		../include/internal_sort.h
		../include/logstream.h
		../include/matrix.h
		../include/mergeheap.h
		../include/mm_base.h
		../include/mm.h
		../include/mm_register.h
		../include/persist.h
		../include/portability.h
		../include/progress_indicator_arrow.h
		../include/progress_indicator_base.h
		../include/progress_indicator_spin.h
		../include/progress_indicator_terminal.h
		../include/progress_indicator_tpie.h
		../include/quicksort.h
		../include/sort_manager.h
		../include/stdio_stack.h
		../include/timer.h
		../include/tpie_assert.h
		../include/tpie_log.h
		../include/tpie_stats_coll.h
		../include/tpie_stats.h
		../include/tpie_stats_stream.h
		../include/tpie_stats_tree.h
		../include/tpie_tempnam.h
		../include/vararray.h
		../include/versions.h
		#
		# in lib/src
		src/lib_config.h
		)

set (AMI_SOURCES
		src/ami_bit_permute.cpp
		src/ami_device.cpp
		src/ami_key.cpp
		src/ami_matrix_blocks.cpp
		src/ami_stream_base.cpp
	)

set (BTE_SOURCES
	src/bte_stream_base.cpp
	)

set (OTHER_SOURCES
	src/bit.cpp
	src/bit_matrix.cpp
	src/cpu_timer.cpp
	src/logstream.cpp
	src/mm_base.cpp
	src/mm_register.cpp
	src/portability.cpp
	src/tpie_log.cpp
	src/tpie_tempnam.cpp
	)

set(AMI ${AMI_HEADERS} ${AMI_SOURCES})
set(BTE ${BTE_HEADERS} ${BTE_SOURCES})
set(OTHER ${OTHER_HEADERS} ${OTHER_SOURCES})

message(${AMI})
message(${BTE})
message(${OTHER})

add_library(tpie ${BTE} ${AMI} ${OTHER})

install(TARGETS tpie DESTINATION lib)
