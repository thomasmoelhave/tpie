# doc/CMakeLists.txt uses configure_file behavior from CMake 2.8
cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0063 NEW)

project(tpie)


#### CONFIG.H Checks:
include(CheckIncludeFiles)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
#### Dependencies
## Boost
set(Boost_USE_MULTITHREADED ON)
if(WIN32)
	set(Boost_USE_STATIC_LIBS ON)
endif(WIN32)
find_package(Boost REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(TPIE_FRACTIONDB_DIR_INL)
  include_directories(${TPIE_FRACTIONDB_DIR_INL})
endif(TPIE_FRACTIONDB_DIR_INL)

check_include_files("unistd.h" TPIE_HAVE_UNISTD_H)
check_include_files("sys/unistd.h" TPIE_HAVE_SYS_UNISTD_H)


option(TPIE_USE_GIT_COMMIT "Use git commit" ON)
if(TPIE_USE_GIT_COMMIT)
	# Ryan Pavlik's Git revision description helper
	# http://stackoverflow.com/a/4318642
	include(GetGitRevisionDescription)	
	get_git_head_revision(GIT_REFSPEC GIT_COMMIT)
else(TPIE_USE_GIT_COMMIT)
	set(GIT_REFSPEC "no_git_ref")
	set(GIT_COMMIT "no_git_commit")
endif()

## Snappy
option(TPIE_USE_SNAPPY "Use Snappy, a fast compressor/decompressor" ON)
if(TPIE_USE_SNAPPY)
	find_package(Snappy)
	if(${SNAPPY_FOUND})
		message(STATUS "Snappy found")
		set(TPIE_HAS_SNAPPY ON)
	else(${SNAPPY_FOUND})
		message(WARNING "Snappy NOT found, TPIE will NOT compress intermediate sort streams")
		set(TPIE_HAS_SNAPPY OFF)
	endif(${SNAPPY_FOUND})
endif(TPIE_USE_SNAPPY)

## LZ4
option(TPIE_USE_LZ4 "Use LZ4, Extremely Fast Compression algorithm" ON)
if(TPIE_USE_LZ4)
	find_package(LZ4)
	if(${LZ4_FOUND})
		set(TPIE_HAS_LZ4 ON)
	else(${LZ4_FOUND})
		set(TPIE_HAS_LZ4 OFF)
	endif(${LZ4_FOUND})
endif(TPIE_USE_LZ4)

## ZSTD
option(TPIE_USE_ZSTD "Use ZSTD, Compression algorithm" ON)
if(TPIE_USE_ZSTD)
	find_package(Zstd)
	if(${ZSTD_FOUND})
		set(TPIE_HAS_ZSTD ON)
	else(${ZSTD_FOUND})
		set(TPIE_HAS_ZSTD OFF)
	endif(${ZSTD_FOUND})
endif(TPIE_USE_ZSTD)

option(TPIE_SHARED "Build tpie as a shared library" OFF)
option(TPIE_EXECUTION_TIME_PREDICTOR "Enable execution time predictor" OFF)

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

if (CMAKE_BUILD_TYPE)
  string(TOUPPER ${CMAKE_BUILD_TYPE} bt)
  string(REGEX MATCH " -DNDEBUG " TPIE_NDEBUG " ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${bt}} ")
else()
  option(TPIE_NDEBUG "Disable debugging information" ON)
endif()

set(TPIE_S ${CMAKE_CURRENT_SOURCE_DIR})
set(TPIE_B ${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tpie/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/tpie/config.h)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tpie/config.h DESTINATION ${HEADERS_INSTALL_DIR})
add_subdirectory(tpie)

add_subdirectory(doc)

option(COMPILE_TEST "Compile test programs" ON)
option(TPL_LOGGING "Enable tpie logging." ON)
option(TPIE_DEPRECATED_WARNINGS "Enable warnings for deprecated classes, methods and typedefs" OFF)
option(TPIE_PARALLEL_SORT "Enable parallel quick sort implementation" ON)

if (COMPILE_TEST)
	ENABLE_TESTING()
	add_subdirectory(test)
	add_subdirectory(apps)
endif (COMPILE_TEST)

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_CONTACT "rav@cs.au.dk")
set(CPACK_GENERATOR TGZ)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TPIE")
set(CPACK_PACKAGE_VENDOR "The TPIE maintainers")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.md")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "TPIE ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")
if(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  #set(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} ${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
  set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\thomasmoelhave.github.com/tpie/")
  set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\thomasmoelhave.github.com/tpie/")
  set(CPACK_NSIS_CONTACT ${CPACK_PACKAGE_CONTACT})
  set(CPACK_NSIS_MODIFY_PATH ON)
  list(APPEND CPACK_GENERATOR NSIS)
else(WIN32 AND NOT UNIX)
  list(APPEND CPACK_GENERATOR DEB)
endif(WIN32 AND NOT UNIX)

install(DIRECTORY tpie
	DESTINATION include
	FILES_MATCHING REGEX "\\.h$|\\.inl$"
	PATTERN "deadcode" EXCLUDE)

install(DIRECTORY share/tpie
	DESTINATION share)

include(CPack)
