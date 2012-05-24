set (COMMON_DEPS
		app_config.h
		app_config.cpp
		getopts.cpp 
		getopts.h
		parse_args.cpp
		parse_args.h
	)

add_library(test_common ${COMMON_DEPS})
set_target_properties(test_common PROPERTIES FOLDER tpie/test)

macro(add_tpie_test NAME)
	add_executable(${NAME} ${NAME}.cpp) 
	set_target_properties(${NAME} PROPERTIES FOLDER tpie/test)
	target_link_libraries(${NAME} tpie test_common )
endmacro(add_tpie_test)

add_tpie_test(test_ami_stack)
add_tpie_test(test_sort)
add_tpie_test(test_big_scan)
add_tpie_test(test_ami_queue)

add_subdirectory(speed_regression)

add_subdirectory(unit)

add_executable(tpiecat cat.cpp)
set_target_properties(tpiecat PROPERTIES FOLDER tpie/test)
target_link_libraries(tpiecat tpie)
