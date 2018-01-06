# LZ4, Extremely Fast Compression algorithm
find_path(LZ4_INCLUDE_DIR NAMES lz4.h)
find_library(LZ4_LIBRARY NAMES lz4)
find_package_handle_standard_args(LZ4 DEFAULT_MSG LZ4_LIBRARY LZ4_INCLUDE_DIR)
if (LZ4_FOUND)
  add_library(lz4 SHARED IMPORTED)
  set_property(TARGET lz4 PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${LZ4_INCLUDE_DIR})
  set_property(TARGET lz4 PROPERTY IMPORTED_LOCATION ${LZ4_LIBRARY})
endif()
