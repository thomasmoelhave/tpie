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

else (DOXYGEN)
    message(STATUS "Doxygen not found, API documentation cannot be generated.") 
endif(DOXYGEN)
