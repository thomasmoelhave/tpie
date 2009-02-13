#Path to output doxygen configuration file
SET(DOXYFILE ${PROJECT_BINARY_DIR}/doc/api/Doxyfile)

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

    ADD_CUSTOM_TARGET(apidocs ${DOXYGEN} ${DOXYFILE})
else (DOXYGEN)
    message(STATUS "Doxygon not found, api documentation can not be generated.") 
endif(DOXYGEN)

#CMake should process the Doxyfile.in file replacing
#variables placing the result in the build directory
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
               ${DOXYFILE}
               @ONLY)

