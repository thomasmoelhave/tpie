add_executable(helloworld helloworld.cpp)
set_target_properties(helloworld PROPERTIES FOLDER tpie)
target_link_libraries(helloworld tpie)
