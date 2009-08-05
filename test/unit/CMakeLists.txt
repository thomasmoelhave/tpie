add_executable(test_bte test_bte.cpp)
target_link_libraries(test_bte tpie)

if(NOT WIN32) 
set(BTES ${BTES} ufs mmap)
endif(NOT WIN32)
set(BTES ${BTES} cache stdio)

foreach(bte ${BTES})
  foreach(test basic randomread)
    add_test(bte_${bte}_${test} test_bte ${bte} ${test})
  endforeach(test)
endforeach(bte)
