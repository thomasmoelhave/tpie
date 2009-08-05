cmake_minimum_required(VERSION 2.6)

project(tpie)

include_directories(.)
add_definitions(-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

#### CONFIG.H Checks:
include(CheckIncludeFiles)

if(NOT WIN32) 
	add_definitions("-Wall -Wextra")
endif(NOT WIN32)

check_include_files("unistd.h" TPIE_HAVE_UNISTD_H)
check_include_files("sys/unistd.h" TPIE_HAVE_SYS_UNISTD_H)

#### Installation paths
#Default paths
set(BIN_INSTALL_DIR bin)
set(LIB_INSTALL_DIR lib)
set(HEADERS_INSTALL_DIR include/tpie)
if (WIN32)
	set(DOC_INSTALL_DIR doc)
else(WIN32)
	set(DOC_INSTALL_DIR "share/doc/tpie")
endif(WIN32)

set(INSTALL_TARGETS_DEFAULT_ARGS
	RUNTIME DESTINATION ${BIN_INSTALL_DIR}
	LIBRARY DESTINATION ${LIB_INSTALL_DIR}
	ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tpie/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/tpie/config.h)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tpie/config.h DESTINATION ${HEADERS_INSTALL_DIR})
add_subdirectory(tpie)


add_subdirectory(doc)

option(COMPILE_TEST "Compile test programs" ON)
option(TPIE_USE_EXCEPTIONS "Throw exceptions on certain errors." OFF)
option(DEBUG_ASSERTIONS "Enable tpie_assert." ON)
option(TPL_LOGGING "Enable tpie logging." ON)

if (COMPILE_TEST)
	ENABLE_TESTING()
	add_subdirectory(test)
	add_subdirectory(apps)
endif (COMPILE_TEST)


include(CPack)


