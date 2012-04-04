#Path to output doxygen configuration file
SET(DOXYFILE ${PROJECT_BINARY_DIR}/doc/Doxyfile)

#Search for doxygen
find_package(Doxygen)

if (DOXYGEN)
#If Doxygen is present, add docs custom target
#that executes doxygen

option(INTERNAL_DOCS "Enable internal documentation comments in the doxygen output" ON)

#This variable is automatically replaced when configuring Doxyfile.in.
if (INTERNAL_DOCS)
	set(USE_INTERNAL_DOCS "YES")
else (INTERNAL_DOCS)
	set(USE_INTERNAL_DOCS "NO")
endif(INTERNAL_DOCS)

ADD_CUSTOM_TARGET(apidocs
	sh -c "sed -e \"s/@VERSION@/`git describe --always`/g\" ${PROJECT_SOURCE_DIR}/doc/header.html > ${PROJECT_BINARY_DIR}/doc/header.html"
	COMMAND sh -c "sed -e \"s/@VERSION@/`git describe --always`/g\" ${PROJECT_SOURCE_DIR}/doc/footer.html > ${PROJECT_BINARY_DIR}/doc/footer.html"
	COMMAND ${DOXYGEN} ${DOXYFILE}
	VERBATIM)

#CMake should process the Doxyfile.in file replacing
#variables placing the result in the build directory
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${DOXYFILE} @ONLY)

set(S ${CMAKE_CURRENT_SOURCE_DIR})
set(C ${S}/code)
set(B ${CMAKE_CURRENT_BINARY_DIR})

file(READ ${C}/file_stream.inl DOCCODE_FILE_STREAM)
configure_file(${S}/file_stream.dox.in ${B}/file_stream.dox @ONLY)

file(READ ${C}/fractiondb.inl DOCCODE_FRACTIONDB)
configure_file(${S}/fractiondb.dox.in ${B}/fractiondb.dox @ONLY)

file(READ ${C}/memory.inl DOCCODE_MEMORY)
file(READ ${C}/memory2.inl DOCCODE_MEMORY2)
configure_file(${S}/memory.dox.in ${B}/memory.dox @ONLY)

file(READ ${C}/priority_queue.inl DOCCODE_PRIORITY_QUEUE)
configure_file(${S}/priority_queue.dox.in ${B}/priority_queue.dox @ONLY)

file(READ ${C}/progress1.inl DOCCODE_PROGRESS1)
file(READ ${C}/progress2.inl DOCCODE_PROGRESS2)
file(READ ${C}/progress3.inl DOCCODE_PROGRESS3)
configure_file(${S}/progress.dox.in ${B}/progress.dox @ONLY)

file(READ ${C}/sorting_external.inl DOCCODE_SORTING_EXTERNAL)
configure_file(${S}/sorting_external.dox.in ${B}/sorting_external.dox @ONLY)

file(READ ${C}/sorting_internal.inl DOCCODE_SORTING_INTERNAL)
configure_file(${S}/sorting_internal.dox.in ${B}/sorting_internal.dox @ONLY)

else (DOXYGEN)
    message(STATUS "Doxygen not found, API documentation cannot be generated.") 
endif(DOXYGEN)

add_executable(doccode code/code.cpp)
target_link_libraries(doccode tpie ${Boost_LIBRARIES})
