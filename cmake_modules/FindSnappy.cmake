# Snappy, a fast compressor/decompressor
find_path(SNAPPY_INCLUDE_DIR NAMES snappy.h)
find_library(SNAPPY_LIBRARY NAMES snappy)
find_package_handle_standard_args(SNAPPY DEFAULT_MSG SNAPPY_LIBRARY SNAPPY_INCLUDE_DIR)
if (SNAPPY_FOUND)
  add_library(snappy SHARED IMPORTED)
  set_property(TARGET snappy PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${SNAPPY_INCLUDE_DIR})
  set_property(TARGET snappy PROPERTY IMPORTED_LOCATION ${SNAPPY_LIBRARY})
endif()

