set (SPEED_DEPS testtime.h stat.h)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/sysinfo.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/sysinfo.cpp" @ONLY)
list(APPEND SPEED_DEPS "${CMAKE_CURRENT_BINARY_DIR}/sysinfo.cpp" sysinfo.h)

add_executable(stream_speed_test stream.cpp ${SPEED_DEPS})
target_link_libraries(stream_speed_test tpie)
set_target_properties(stream_speed_test PROPERTIES FOLDER tpie/test)

add_executable(progress_speed_test progress.cpp ${SPEED_DEPS})
target_link_libraries(progress_speed_test tpie)
set_target_properties(progress_speed_test PROPERTIES FOLDER tpie/test)

add_executable(sort_speed_test sort.cpp ${SPEED_DEPS})
target_link_libraries(sort_speed_test tpie)
set_target_properties(sort_speed_test PROPERTIES FOLDER tpie/test)

add_executable(pq_speed_test priority_queue.cpp ${SPEED_DEPS})
target_link_libraries(pq_speed_test tpie)
set_target_properties(pq_speed_test PROPERTIES FOLDER tpie/test)

add_executable(array_speed_test array.cpp)
target_link_libraries(array_speed_test tpie)
set_target_properties(array_speed_test PROPERTIES FOLDER tpie/test)
