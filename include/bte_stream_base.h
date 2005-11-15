//
// File: bte_stream_base.h (formerly bte_base_stream.h)
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/11/94
//
// $Id: bte_stream_base.h,v 1.13 2005-11-15 15:41:34 jan Exp $
//
#ifndef _BTE_STREAM_BASE_H
#define _BTE_STREAM_BASE_H

// Get definitions for working with Unix and Windows
#include <portability.h>

#include <persist.h>
// Get the BTE error codes.
#include <bte_err.h>
// Get statistics definitions.
#include <tpie_stats_stream.h>

// Include the registration based memory manager.
#define MM_IMP_REGISTER
#include <mm.h>

// Max length of a stream file name.
#define BTE_STREAM_PATH_NAME_LEN 128

// The magic number of the file storing the stream.
// (in network byteorder, it spells "TPST": TPie STream)
#define BTE_STREAM_HEADER_MAGIC_NUMBER	0x54505354 

// BTE stream types passed to constructors.
enum BTE_stream_type {
    BTE_READ_STREAM = 1, // Open existing stream for reading.
    BTE_WRITE_STREAM,    // Open for read/writing. Create if non-existent.
    BTE_APPEND_STREAM,   // Open for writing at end. Create if needed.
    BTE_WRITEONLY_STREAM // Open only for writing (allows mmb optimization)
    // (must be sequential write through whole file)
};

// BTE stream status. 
enum BTE_stream_status {
    BTE_STREAM_STATUS_NO_STATUS = 0,
    BTE_STREAM_STATUS_INVALID = 1,
    BTE_STREAM_STATUS_EOS_ON_NEXT_CALL,
    BTE_STREAM_STATUS_END_OF_STREAM
};

#include <bte_stream_header.h>
#include <bte_stream_base_generic.h>

// An abstract class template which implements a single stream of objects 
// of type T within the BTE.  This is the superclass of all actual 
// implementations of streams of T within the BTE (e.g. mmap() streams, 
// UN*X file system streams, and kernel streams).
template<class T> 
class BTE_stream_base: public BTE_stream_base_generic {

public:
    BTE_stream_base() {
	// Do nothing.
    };

    // Tell the stream whether to leave its data on the disk or not
    // when it is destructed.
    void persist (persistence p) { 
	m_persistenceStatus = p; 
    }

    // Inquire the persistence status of this BTE stream.
    persistence persist() const { 
	return m_persistenceStatus; 
    }

    // Return true if a read-only stream.
    bool read_only () const { 
	return m_readOnly;
    }

    // Inquire the status.
    BTE_stream_status status() const { 
	return m_status; 
    }

    // Inquire the OS block size.
    TPIE_OS_SIZE_T os_block_size () const {
	return TPIE_OS_BLOCKSIZE();
    }

    const tpie_stats_stream& stats() const { 
	return m_streamStatistics; 
    }

    // Return the path name in newly allocated space.
    BTE_err name (char **stream_name) const;

protected:

    using BTE_stream_base_generic::remaining_streams;
    using BTE_stream_base_generic::gstats_;
  
    // Check the given header for reasonable values.
    int check_header();

    // Initialize the header with as much information as is known here.
    void init_header();

    inline BTE_err register_memory_allocation (TPIE_OS_SIZE_T sz);
    inline BTE_err register_memory_deallocation (TPIE_OS_SIZE_T sz);

    // Record statistics both globally (on base-class level) and
    // locally (on instance level).
    inline void record_statistics(TPIE_STATS_STREAM event);

    // A pointer to the mapped in header block for the stream.
    BTE_stream_header *m_header;

    // The persistence status of this stream.
    persistence       m_persistenceStatus;

    // The status (integrity) of this stream.
    BTE_stream_status m_status;

    // How deeply is this stream nested.
    unsigned int      m_substreamLevel;

    // Non-zero if this stream was opened for reading only.
    bool              m_readOnly; 

    // Statistics for this stream only.
    tpie_stats_stream m_streamStatistics;

    // The size of a physical block.
    TPIE_OS_SIZE_T m_osBlockSize;

    // Offset of the current item in the file. This is the logical
    // offset of the item within the file, that is, the place we would
    // have to lseek() to in order to read() or write() the item if we
    // were using ordinary (i.e. non-mmap()) file access methods.
    TPIE_OS_OFFSET m_fileOffset;

    // Beginning of the file.  Can't write before here.
    TPIE_OS_OFFSET m_logicalBeginOfStream;

    // Offset just past the end of the last item in the stream. If this
    // is a substream, we can't write here or anywhere beyond.
    TPIE_OS_OFFSET m_logicalEndOfStream;

    // Length of the underlying file.
    TPIE_OS_OFFSET m_fileLength;

    // A place to cache OS error values. It is normally set after each
    // call to the OS.
    int m_osErrno;

    //  Name of the underlying file.
    char m_path[BTE_STREAM_PATH_NAME_LEN];

};

template<class T>
int BTE_stream_base<T>::check_header() {

    if (m_header == NULL) {
	TP_LOG_FATAL_ID ("Could not map header.");
	return -1;
    }

    if (m_header->m_magicNumber != BTE_STREAM_HEADER_MAGIC_NUMBER) {
	TP_LOG_FATAL_ID ("header: magic number mismatch (expected/obtained):");
	TP_LOG_FATAL_ID (BTE_STREAM_HEADER_MAGIC_NUMBER);
	TP_LOG_FATAL_ID (m_header->m_magicNumber);
	return -1;
    }

    if (m_header->m_headerLength != sizeof (*m_header)) {
      TP_LOG_FATAL_ID ("header: incorrect header length; (expected/obtained):");
      TP_LOG_FATAL_ID (sizeof (BTE_stream_header));
      TP_LOG_FATAL_ID (m_header->m_headerLength);
      TP_LOG_FATAL_ID ("This could be due to a stream written without 64-bit support.");
      return -1;
    }

    if (m_header->m_version != 2) {
      TP_LOG_FATAL_ID ("header: incorrect version (expected/obtained):");
      TP_LOG_FATAL_ID (2);
      TP_LOG_FATAL_ID (m_header->m_version);
      return -1;
    }

    if (m_header->m_type == 0) {
	TP_LOG_FATAL_ID ("header: type is 0 (reserved for base class).");
	return -1;
    }

    if (m_header->m_itemSize != sizeof (T)) {
	TP_LOG_FATAL_ID ("header: incorrect item size (expected/obtained):");
	TP_LOG_FATAL_ID (sizeof(T));
	TP_LOG_FATAL_ID ((TPIE_OS_LONGLONG)m_header->m_itemSize);
	return -1;
    }

    if (m_header->m_osBlockSize != os_block_size()) {
	TP_LOG_FATAL_ID ("header: incorrect OS block size (expected/obtained):");
	TP_LOG_FATAL_ID ((TPIE_OS_LONGLONG)os_block_size());
	TP_LOG_FATAL_ID ((TPIE_OS_LONGLONG)m_header->m_osBlockSize);
	return -1;
    }

    return 0;
}

template<class T>
void BTE_stream_base<T>::init_header () {
    tp_assert(m_header != NULL, "NULL header pointer");
    m_header->m_magicNumber    = BTE_STREAM_HEADER_MAGIC_NUMBER;
    m_header->m_version        = 2;
    m_header->m_type           = 0; // Not known here.
    m_header->m_headerLength   = sizeof(*m_header);
    m_header->m_itemSize       = sizeof(T);
    m_header->m_osBlockSize    = os_block_size();
    m_header->m_blockSize      = 0; // Not known here.
    m_header->m_itemLogicalEOF = 0;
}

template<class T>
BTE_err BTE_stream_base<T>::register_memory_allocation (TPIE_OS_SIZE_T sz) {

    if (MM_manager.register_allocation(sz) != MM_ERROR_NO_ERROR) {

	m_status = BTE_STREAM_STATUS_INVALID;

	TP_LOG_FATAL_ID("Memory manager error in allocation.");

	return BTE_ERROR_MEMORY_ERROR;
    }
    return BTE_ERROR_NO_ERROR;
}

template<class T>
BTE_err BTE_stream_base<T>::register_memory_deallocation (TPIE_OS_SIZE_T sz) {

    if (MM_manager.register_deallocation (sz) != MM_ERROR_NO_ERROR) {

	m_status = BTE_STREAM_STATUS_INVALID;

	TP_LOG_FATAL_ID("Memory manager error in deallocation.");

	return BTE_ERROR_MEMORY_ERROR;
    }
    return BTE_ERROR_NO_ERROR;
}

// Return the path name in newly allocated space.
template < class T >
BTE_err BTE_stream_base<T>::name (char **stream_name) const {
  
    // O.k. not to use TPIE_OS_SIZE_T
    size_t len = strlen (m_path);

    tp_assert (len < BTE_STREAM_PATH_NAME_LEN, "Path length is too long.");
  
    // Return the path name in newly allocated space.
    char *newPath = new char[len + 1];
    strncpy (newPath, m_path, len + 1);
    *stream_name = newPath;

    return BTE_ERROR_NO_ERROR;
}

template < class T >
inline void BTE_stream_base<T>::record_statistics(TPIE_STATS_STREAM event) {

    //  Record for base class, i.e., globally.
    gstats_.record(event);

    //  Record for instance, i.e., locally.
    m_streamStatistics.record(event);

};

#endif // _BTE_STREAM_BASE_H 
