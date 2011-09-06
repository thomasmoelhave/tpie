add_executable(stream_speed_test stream.cpp testtime.h)
target_link_libraries(stream_speed_test tpie)
set_target_properties(stream_speed_test PROPERTIES FOLDER tpie/test)

add_executable(pq_speed_test priority_queue.cpp)
target_link_libraries(pq_speed_test tpie)
set_target_properties(pq_speed_test PROPERTIES FOLDER tpie/test)
