set (COMMON_DEPS
		app_config.h
		app_config.cpp
		getopts.cpp 
		getopts.h
		parse_args.cpp
		parse_args.h
	)

set (SCAN_DEPS 
		scan_count.cpp
		scan_count.h
		scan_diff.h
		scan_random.cpp
		scan_random.h
		scan_square.h
	#	scan_uniform_sm.cpp
#		scan_uniform_sm.h
		scan_universal.h
	)
add_library(test_common ${COMMON_DEPS})
add_library(test_scan_common ${SCAN_DEPS})

macro(add_tpie_test NAME)
	add_executable(${NAME} ${NAME}.cpp) 
	target_link_libraries(${NAME} tpie test_common test_scan_common )
endmacro(add_tpie_test)

add_tpie_test(test_ami_arith)
add_tpie_test(test_ami_gp)
#add_tpie_test(test_ami_merge)
#add_tpie_test(test_ami_sm)
add_tpie_test(test_ami_stack)
add_tpie_test(test_sort)
#add_tpie_test(test_ami_bp)
#add_tpie_test(test_ami_matrix)
#add_tpie_test(test_ami_pmerge)
add_tpie_test(test_ami_sort24)
add_tpie_test(test_big_scan)
add_tpie_test(test_ami_btree)
#add_tpie_test(test_ami_matrix_pad)
add_tpie_test(test_ami_queue)
add_tpie_test(test_ami_sort)
add_tpie_test(test_correctness)
add_tpie_test(test_priority_queue)

if(NOT WIN32)
  add_subdirectory(speed_regression)
endif(NOT WIN32)

add_subdirectory(unit)
