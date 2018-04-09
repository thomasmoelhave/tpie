add_executable(siv_dump siv_dump.cpp)
set_target_properties(siv_dump PROPERTIES FOLDER tpie)
target_link_libraries(siv_dump tpie)
