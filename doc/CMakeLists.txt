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

set(TPIE_DOCS ${CMAKE_CURRENT_SOURCE_DIR})
set(TPIE_DOCB ${CMAKE_CURRENT_BINARY_DIR})
set(TPIE_DOCC ${TPIE_DOCS}/code)

file(READ ${TPIE_DOCC}/file_stream.inl DOCCODE_FILE_STREAM)
configure_file(${TPIE_DOCS}/file_stream.dox.in ${TPIE_DOCB}/file_stream.dox @ONLY)

file(READ ${TPIE_DOCC}/fractiondb.inl DOCCODE_FRACTIONDB)
configure_file(${TPIE_DOCS}/fractiondb.dox.in ${TPIE_DOCB}/fractiondb.dox @ONLY)

file(READ ${TPIE_DOCC}/memory.inl DOCCODE_MEMORY)
file(READ ${TPIE_DOCC}/memory2.inl DOCCODE_MEMORY2)
configure_file(${TPIE_DOCS}/memory.dox.in ${TPIE_DOCB}/memory.dox @ONLY)

file(READ ${TPIE_DOCC}/priority_queue.inl DOCCODE_PRIORITY_QUEUE)
configure_file(${TPIE_DOCS}/priority_queue.dox.in ${TPIE_DOCB}/priority_queue.dox @ONLY)

file(READ ${TPIE_DOCC}/progress1.inl DOCCODE_PROGRESS1)
file(READ ${TPIE_DOCC}/progress2.inl DOCCODE_PROGRESS2)
file(READ ${TPIE_DOCC}/progress3.inl DOCCODE_PROGRESS3)
configure_file(${TPIE_DOCS}/progress.dox.in ${TPIE_DOCB}/progress.dox @ONLY)

file(READ ${TPIE_DOCC}/sorting_external.inl DOCCODE_SORTING_EXTERNAL)
configure_file(${TPIE_DOCS}/sorting_external.dox.in ${TPIE_DOCB}/sorting_external.dox @ONLY)

file(READ ${TPIE_DOCC}/sorting_internal.inl DOCCODE_SORTING_INTERNAL)
configure_file(${TPIE_DOCS}/sorting_internal.dox.in ${TPIE_DOCB}/sorting_internal.dox @ONLY)

else (DOXYGEN)
    message(STATUS "Doxygen not found, API documentation cannot be generated.") 
endif(DOXYGEN)

add_executable(doccode code/code.cpp)
target_link_libraries(doccode tpie ${Boost_LIBRARIES})

set(TPIE_DOCH ${TPIE_DOCB}/html)
configure_file(${TPIE_DOCS}/madalgo-doxygen.css ${TPIE_DOCH}/madalgo-doxygen.css COPYONLY)
configure_file(${TPIE_DOCS}/printstyle.css ${TPIE_DOCH}/printstyle.css COPYONLY)
