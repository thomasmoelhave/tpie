configure_file("${CMAKE_CURRENT_SOURCE_DIR}/tpie_test_paths.h.in" "${CMAKE_CURRENT_BINARY_DIR}/tpie_test_paths.h" @ONLY)

macro(add_unittest NAME)
  add_executable(ut-${NAME} test_${NAME}.cpp common.h merge_sort.h priority_queue.h test_timer.h)
  set_target_properties(ut-${NAME} PROPERTIES FOLDER tpie/unittest)
  target_link_libraries(ut-${NAME} tpie Boost::boost)
  target_include_directories(ut-${NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  if(TPIE_HAS_SNAPPY)
    target_link_libraries(ut-${NAME} ${Snappy_LIBRARY})
  endif(TPIE_HAS_SNAPPY)
  if(TPIE_HAS_LZ4)
	  target_link_libraries(ut-${NAME} ${LZ4_LIBRARY})
  endif(TPIE_HAS_LZ4)
  set(MTESTS ${ARGV})
  list(REMOVE_AT MTESTS 0)
  foreach(TEST ${MTESTS})
    add_test(${NAME}_${TEST} ut-${NAME} ${TEST})
  endforeach(TEST)
endmacro(add_unittest)

macro(add_fulltest NAME)
  set(MTESTS ${ARGV})
  list(REMOVE_AT MTESTS 0)
  foreach(TEST ${MTESTS})
    add_test(NAME ${NAME}_${TEST} CONFIGURATIONS FullTest COMMAND ut-${NAME} ${TEST})
  endforeach(TEST)
endmacro(add_fulltest)

add_unittest(allocator deque list)
add_unittest(ami_stream basic truncate)
add_unittest(array
	basic
	iterators
	unique_ptr
	memory
	bit_basic
	bit_iterators
	bit_memory
	copyempty
	arrayarray
	frontback
	swap
	allocator
	copy
	from_view
	assign
	)
add_unittest(block_collection basic erase overwrite)
add_unittest(block_collection_cache basic erase overwrite)
add_unittest(compressed_stream
	basic seek seek_2 reopen_1 reopen_2 read_seek
	truncate truncate_2 position_0 position_1 position_2 position_3
	position_4 position_5 position_6 position_7
	position_8 position_9
	position_seek uncompressed uncompressed_new
	backwards read_back_seek read_back_seek_2 read_back_throw

	basic_u seek_u seek_2_u reopen_1_u reopen_2_u read_seek_u
	truncate_u truncate_2_u position_0_u position_1_u position_2_u
	position_3_u position_4_u position_5_u position_6_u position_7_u
	position_8_u position_9_u
	position_seek_u uncompressed_u uncompressed_new_u
	backwards_u read_back_seek_u read_back_seek_2_u read_back_throw_u

	backwards_fs

	odd_block_size write_only
	write_peek

	lockstep_reverse
)
add_unittest(btree
	internal_augment
	internal_basic
	internal_build
	internal_static
	internal_unordered
	internal_bound
	internal_iterator
	internal_key_and_compare

	external_augment
	external_basic
	external_bound
	external_build
	external_iterator
	external_key_and_compare
	external_reopen
	external_static_reopen
    external_static_iterator

	serialized_build
	serialized_reopen
    serialized_iterator
    serialized_lz4_build
    serialized_lz4_reopen
    serialized_read_old_format
    serialized_snappy_build
    serialized_snappy_reopen
    serialized_zstd_build
    serialized_zstd_reopen
	)
	
add_unittest(disjoint_set basic memory)
add_unittest(external_priority_queue basic parameters remove_group_buffer)
add_unittest(external_queue basic empty_size sized large)
add_unittest(external_sort amismall small tiny)
add_unittest(external_stack new named-new ami named-ami io)
add_unittest(file_count basic)
add_unittest(filestream memory)
add_unittest(freespace_collection alloc size)
add_unittest(internal_priority_queue basic memory)
add_unittest(internal_queue basic memory)
add_unittest(internal_stack basic memory)
add_unittest(internal_vector basic memory)
add_unittest(job repeat)
add_unittest(memory basic)
add_unittest(merge_sort
	empty_input
	internal_report
	internal_report_after_resize
	one_run_external_report
	external_report
	small_final_fanout
	evacuate_before_merge
	evacuate_before_report
	sort_upper_bound
	sort_faulty_upper_bound
	temp_file_usage
	tall_tree
	)
add_unittest(packed_array basic1 basic2 basic4)
add_unittest(parallel_sort basic1 basic2 general equal_elements bad_case)
add_unittest(serialization serialization2 stream stream_dtor stream_reopen stream_reverse stream_temp)
add_unittest(serialization_sort
	empty_input
	internal_report
	internal_report_after_resize
	one_run_external_report
	external_report
	small_final_fanout
	evacuate_before_merge
	evacuate_before_report
	file_limit
	)
add_unittest(stats simple)
add_unittest(stream
	basic
	array
	array_compressed
	odd
	odd_compressed
	reopen
	truncate
	extend
	backwards
	array_file
	odd_file
	truncate_file
	extend_file
	backwards_file
	user_data
	user_data_file
	peek_skip_1
	peek_skip_2
	backwards_compressed
	extend_compressed
	truncate_compressed
	user_data_compressed
	)
add_unittest(stream_exception basic)
add_unittest(pipelining
	vector
	filestream
	fspull
	merge
	reverse
	internal_reverse
	passive_reverse
	internal_passive_reverse
	sort
	sorttrivial
	operators
	uniq
	memory
	fork
	merger_memory
	bound_fetch_forward
	fetch_forward
	forward_multiple_pipelines
	forward_unique_ptr
	pipe_base_forward
	virtual
	virtual_cref_item_type
	virtual_fork
	prepare
	end_time
	pull_iterator
	push_iterator
	parallel
	parallel_ordered
	parallel_multiple
	parallel_own_buffer
	parallel_push_in_end
	node_map
	join
	split
	copy_ctor
	datastructures
	node_map
	subpipeline
	file_limit_sort
	passive_virtual_management
	join_split_dealloc
	nodeset_dealloc
	pipeline_dealloc
	parallel_exception
	parallel_exception_2
	exception
	subpipeline_exception
	subpipeline_exception2
	pull
	virtual_pull
	devirtualize
	devirtualize_pull
	)
add_unittest(pipelining_runtime evacuate get_phase_graph optimal_satisfiable_ordering evacuate_phase_graph)
add_unittest(pipelining_serialization basic reverse sort)
add_unittest(maybe basic unique_ptr)
add_unittest(close_file
	internal
	serialization_writer_close
	serialization_writer_dtor
	serialization_reader_dtor
	file_limit_enforcement
)
add_unittest(node_name gcc msvc)
add_unittest(snappy basic)

add_unittest(tiny sort set map multiset multimap)

add_unittest(raw_file_accessor open_rw_new try_open_rw)

add_fulltest(ami_stream stress)
add_fulltest(disjoint_set large large_cycle very_large medium ovelflow stress)
add_fulltest(external_priority_queue medium large large_cycle memory overflow very_large)
add_fulltest(external_sort amilarge large tall very_large)
add_fulltest(hashmap speed)
add_fulltest(internal_priority_queue large_cycle)
add_fulltest(internal_vector medium large)
add_fulltest(memory parallel parallel_malloc parallel_stdnew)
add_fulltest(parallel_sort general2 large_item stress_test)
add_fulltest(pipelining sortbig parallel_step)
add_fulltest(stream stress stress_compressed stress_file)
