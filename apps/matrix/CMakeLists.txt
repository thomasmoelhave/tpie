SET (COMMON_DEPS
    app_config.h
    app_config.cpp
    fill_upper_tri.h
    getopts.h
    getopts.cpp
    matrix.h
    matrix_fill.h
    matrix_blocks.h
    matrix_blocks.cpp
    matrix_pad.h
    mm_matrix.h
    mm_matrix_base.h
    mm_submatrix.h
    mm_rowref.h
    mm_colref.h
    parse_args.h
    parse_args.cpp
    scan_count.h
    scan_count.cpp
)

add_library(matrix_common ${COMMON_DEPS})

add_executable(test_matrix test_matrix.cpp)
add_executable(test_matrix_pad test_matrix_pad.cpp)

target_link_libraries(test_matrix tpie matrix_common)
target_link_libraries(test_matrix_pad tpie matrix_common)



