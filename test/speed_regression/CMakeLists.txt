add_executable(stream_speed_test stream.cpp testtime.h)
target_link_libraries(stream_speed_test tpie)
set_target_properties(stream_speed_test PROPERTIES FOLDER tpie/test)

add_executable(progress_speed_test progress.cpp testtime.h)
target_link_libraries(progress_speed_test tpie)
set_target_properties(progress_speed_test PROPERTIES FOLDER tpie/test)

add_executable(sort_speed_test sort.cpp testtime.h)
target_link_libraries(sort_speed_test tpie)
set_target_properties(sort_speed_test PROPERTIES FOLDER tpie/test)

add_executable(pq_speed_test priority_queue.cpp testtime.h)
target_link_libraries(pq_speed_test tpie)
set_target_properties(pq_speed_test PROPERTIES FOLDER tpie/test)
