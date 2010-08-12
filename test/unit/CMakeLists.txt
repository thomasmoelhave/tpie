add_executable(test_bte test_bte.cpp)
target_link_libraries(test_bte tpie)

add_executable(test_array test_array.cpp)
target_link_libraries(test_array tpie)

add_executable(streaming streaming.cpp)
target_link_libraries(streaming tpie)


if(NOT WIN32) 
set(BTES ${BTES} ufs mmap)
endif(NOT WIN32)
set(BTES ${BTES} ami_stream cache stdio)

foreach(test source sink sort) 
  add_test(streaming_${test} streaming ${test})
endforeach(test)
add_test(array_basic test_array basic)
add_test(array_iterators test_array iterators)
add_test(array_memory test_array memory)

foreach(bte ${BTES})
  foreach(test basic randomread array)
    add_test(bte_${bte}_${test} test_bte ${bte} ${test})
  endforeach(test)
endforeach(bte)

