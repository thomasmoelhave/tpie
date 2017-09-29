# LZ4, Extremely Fast Compression algorithm

include(LibFindMacros)

find_path(ZSTD_INCLUDE_DIR
        NAMES zstd.h
        )

find_library(ZSTD_LIBRARY
        NAMES zstd
        )

set(ZSTD_PROCESS_INCLUDES ZSTD_INCLUDE_DIR)
set(ZSTD_PROCESS_LIBS ZSTD_LIBRARY)

libfind_process(ZSTD)
