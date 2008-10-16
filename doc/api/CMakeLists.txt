#Path to output doxygen configuration file
SET(DOXYFILE ${PROJECT_BINARY_DIR}/doc/api/Doxyfile)

#CMake should process the Doxyfile.in file replacing
#variables placing the result in the build directory
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
               ${DOXYFILE}
               @ONLY)

#Search for doxygen
find_package(Doxygen)

if (DOXYGEN)
    #If Doxygen is present, add docs custom target
    #that executes doxygen
    ADD_CUSTOM_TARGET(docs ${DOXYGEN} ${DOXYFILE})
else (DOXYGEN)
    message(STATUS "Doxygon not found, api documentation can not be generated.") 
endif(DOXYGEN)
