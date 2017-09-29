# LZ4, Extremely Fast Compression algorithm

include(LibFindMacros)

find_path(LZ4_INCLUDE_DIR
        NAMES lz4.h
        )

find_library(LZ4_LIBRARY
        NAMES lz4
        )

set(LZ4_PROCESS_INCLUDES LZ4_INCLUDE_DIR)
set(LZ4_PROCESS_LIBS LZ4_LIBRARY)

libfind_process(LZ4)
