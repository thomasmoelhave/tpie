add_executable(helloworld helloworld.cpp fib.h)
set_target_properties(helloworld PROPERTIES FOLDER tpie/apps)
target_link_libraries(helloworld tpie)
