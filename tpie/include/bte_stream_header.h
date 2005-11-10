#ifndef _BTE_STREAM_HEADER_H
#define _BTE_STREAM_HEADER_H

// BTE stream header info.
class BTE_stream_header {
public:
  
    // Unique header identifier. Set to BTE_STREAM_HEADER_MAGIC_NUMBER.
    unsigned int magic_number;
    // Should be 2 for current version (version 1 has been deprecated).
    unsigned int version;
    // The type of BTE_STREAM that created this header. Not all types of
    // BTE's are readable by all BTE implementations. For example,
    // BTE_STREAM_STDIO streams are not readable by either
    // BTE_STREAM_UFS or BTE_STREAM_MMAP implementations. The value 0 is
    // reserved for the base class. Use numbers bigger than 0 for the
    // various implementations.
    unsigned int type;
    // The number of bytes in this structure.
    TPIE_OS_SIZE_T header_length;
    // The size of each item in the stream.
    TPIE_OS_SIZE_T item_size;
    // The size of a physical block on the device this stream resides.
    TPIE_OS_SIZE_T os_block_size;
    // Size in bytes of each logical block, if applicable.
    TPIE_OS_SIZE_T block_size;
    // For all intents and purposes, the length of the stream in number
    // of items.
    TPIE_OS_OFFSET item_logical_eof;
};

#endif // _BTE_STREAM_HEADER_H

