# ZSTD, Extremely Fast Compression algorithm
find_path(ZSTD_INCLUDE_DIR NAMES zstd.h)
find_library(ZSTD_LIBRARY NAMES zstd)
find_package_handle_standard_args(ZSTD DEFAULT_MSG ZSTD_LIBRARY ZSTD_INCLUDE_DIR)
if (ZSTD_FOUND)
  add_library(zstd SHARED IMPORTED)
  set_property(TARGET zstd PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${ZSTD_INCLUDE_DIR})
  set_property(TARGET zstd PROPERTY IMPORTED_LOCATION ${ZSTD_LIBRARY})
endif()
