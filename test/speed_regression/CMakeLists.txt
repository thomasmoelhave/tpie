add_executable(stream stream.cpp testtime.h)
target_link_libraries(stream tpie)


add_executable(test22 test.cpp testtime.h)
target_link_libraries(test22 tpie)