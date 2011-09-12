macro(add_unittest NAME)
  add_executable(ut-${NAME} test_${NAME}.cpp)
  set_target_properties(ut-${NAME} PROPERTIES FOLDER tpie/unittest)
  target_link_libraries(ut-${NAME} tpie ${Boost_LIBRARIES})
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


if(Boost_FOUND)
  add_unittest(hashmap chaining linear_probing iterators memory)
endif()
add_unittest(memory basic)
add_unittest(internal_queue basic memory)
add_unittest(internal_stack basic memory)
add_unittest(internal_vector basic memory)
add_unittest(parallel_sort basic1 equal_elements)

add_unittest(external_priority_queue basic)
add_unittest(serialization unsafe safe)
add_fulltest(external_priority_queue medium large large_cycle)

add_unittest(internal_priority_queue basic memory)
add_fulltest(internal_priority_queue large_cycle)

#disable auto_ptr array test for now
#add_unittest(array basic iterators segmented memory auto_ptr memory_segmented bit_basic bit_iterators bit_memory)
add_unittest(array basic iterators segmented memory auto_ptr memory_segmented bit_basic bit_iterators bit_memory)

add_unittest(disjoint_set basic memory)

add_unittest(external_sort small)
add_fulltest(external_sort large)

add_unittest(external_stack small named small-ami named-ami)

add_unittest(stream basic)

add_unittest(ami_stream basic)

#add_executable(test_bte test_bte.cpp)
#set_target_properties(test_bte PROPERTIES FOLDER tpie/unittest)
#target_link_libraries(test_bte tpie)

#if(NOT WIN32) 
#set(BTES ${BTES} ufs mmap)
#endif(NOT WIN32)
#set(BTES ${BTES} ami_stream cache stdio)

#foreach(bte ${BTES})
#  foreach(test basic randomread array)
#    add_test(bte_${bte}_${test} test_bte ${bte} ${test})
#  endforeach(test)
#endforeach(bte)

