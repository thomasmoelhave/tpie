//
// File:    bte_coll_base.h
// Authors: Octavian Procopiuc <tavi@cs.duke.edu>
//          (using some code by Rakesh Barve)
//
// $Id: bte_coll_base.h,v 1.12 2002-01-27 23:34:26 tavi Exp $
//
// BTE_collection_base class and various basic definitions.
//

#ifndef _BTE_COLL_BASE_H
#define _BTE_COLL_BASE_H

#include <stdlib.h>
// For errno
#include <errno.h>
// For strerror()
#include <string.h>

// Include the registration based memory manager.
#define MM_IMP_REGISTER
#include <mm.h>

// For persist.
#include <persist.h>
// For stdio_stack.
#include <stdio_stack.h>
// For BTE_err.
#include <bte_err.h>
// For class tpie_stats_collection.
#include <tpie_stats_coll.h>

// BTE_COLLECTION types passed to constructors.
enum BTE_collection_type {
  BTE_READ_COLLECTION = 1,    // Open existing stream read only.
  BTE_WRITE_COLLECTION,	      // Open for read/write. Create if non-existent.
  BTE_WRITE_NEW_COLLECTION    // Open for read/write a new collection,
                              // even if a nonempty file with that name exists.
};

// BTE collection status.
enum BTE_collection_status {
  BTE_COLLECTION_STATUS_VALID = 0,
  BTE_COLLECTION_STATUS_INVALID = 1,
};

// Maximum length of the file names.
#define BTE_COLLECTION_PATH_NAME_LEN 128

// Number of bytes in the header's user_data_ field.
#define BTE_COLLECTION_USER_DATA_LEN 512

// The magic number of the files storing blocks.
// (in network byteorder, it spells "TPBC": TPie Block Collection)
#define BTE_COLLECTION_HEADER_MAGIC_NUMBER 0x54504243

// Default file name suffixes
#define BTE_COLLECTION_BLK_SUFFIX ".blk"
#define BTE_COLLECTION_STK_SUFFIX ".stk"


// The in-memory representation of the BTE_COLLECTION header.
// This data structure is read from/written to the first 
// (physical) page of the blocks file.

class BTE_collection_header {
public:

  // Unique header identifier. Set to BTE_COLLECTION_HEADER_MAGIC_NUMBER
  unsigned int magic_number;
  // Should be 1 for current version.
  unsigned int version;
  // The type of BTE_COLLECTION that created this header. Setting this
  // field is optional and is mostly for information purposes and
  // similarity with stream header. The current implementations all
  // use the same file format; it's not important to differentiate
  // among them, since they can all read each other's collections. If
  // used, it should be set to a non-zero value (zero is reserved for
  // the base class).
  unsigned int type;
  // The number of bytes in this structure.
  size_t header_length;
  // The number of blocks consumed by this collection, plus 1.
  size_t total_blocks;
  // The highest bid any block of this block collection has, PLUS 1
  // (always <= total_blocks).
  size_t last_block; 
  // The number of valid blocks in this block collection.
  size_t used_blocks;
  // The size of a physical block on the device this stream resides.
  size_t os_block_size;
  // Size in bytes of each logical block.
  size_t block_size;
  // Some data to be filled by the user of the collection.
  char user_data[BTE_COLLECTION_USER_DATA_LEN];
  
  // Default constructor.
  BTE_collection_header();
};

// Setting this to 1 causes the use of ftruncate(2) for extending
// files, which, in conjunction with mmap(2), results in more
// fragmented files and, consequently, slower I/O. See mmap(2) on
// FreeBSD for an explanation. When set to 0, lseek(2) and write(2)
// are used to extend the files.
#define USE_FTRUNCATE 0


// A base class for all implementations of block collection classes.
class BTE_collection_base {
protected:
  
  // An stdio_stack of off_t's.
  stdio_stack<off_t> *freeblock_stack_; 

  // File descriptor for the file backing the block collection.
  int bcc_fd_;

  char base_file_name_[BTE_COLLECTION_PATH_NAME_LEN];

  // Various parameters (will be stored into the file header block).
  BTE_collection_header header_;

  size_t os_block_size_;

  // Persistency flag. Set during construction and using the persist()
  // method.
  persistence per_;

  // Status of the collection. Set during construction.
  BTE_collection_status status_;

  // Read-only flag. Set during construction.
  bool read_only_;

  // Number of blocks from this collection that are currently in memory
  size_t in_memory_blocks_;

  // File pointer position. A value of -1 signals unknown position.
  off_t file_pointer;

  // Statistics for this object.
  tpie_stats_collection stats_;

  // Global collection statistics.
  static tpie_stats_collection gstats_;

private:
  // Helper functions. We don't want them inherited.

  // Initialization common to all constructors.
  void shared_init(BTE_collection_type type, size_t logical_block_factor);

  // Read header from disk.
  BTE_err read_header(char *bcc_name);

  // Write header to disk.
  BTE_err write_header(char* bcc_name);

  void remove_stack_file();

protected:

  // Needs to be inlined!
  BTE_err register_memory_allocation(size_t sz) {
    if (MM_manager.register_allocation(sz) != MM_ERROR_NO_ERROR) {
      status_ = BTE_COLLECTION_STATUS_INVALID;
      LOG_FATAL_ID("Memory manager error in allocation.");
      return BTE_ERROR_MEMORY_ERROR;
    }
    return BTE_ERROR_NO_ERROR;
  }

  // Needs to be inlined!
  BTE_err register_memory_deallocation(size_t sz) {
    if (MM_manager.register_deallocation(sz) != MM_ERROR_NO_ERROR) {
      status_ = BTE_COLLECTION_STATUS_INVALID;
      LOG_FATAL_ID("Memory manager error in deallocation.");
      return BTE_ERROR_MEMORY_ERROR;
    }
    return BTE_ERROR_NO_ERROR;
  }

  off_t bid_to_file_offset(off_t bid) const 
    { return header_.os_block_size + header_.block_size * (bid-1); }

  void create_stack();

  // Common code for all new_block implementations. Inlined.
  BTE_err new_block_getid(off_t& bid) {
    // We try getting a free bid from the stack first. If there aren't
    // any there, we will try to get one after last_block; if there are
    // no blocks past last_block, we will ftruncate() some more blocks
    // to the tail of the BCC and then get a free bid.
    off_t *lbn;
    BTE_err err;
    if (header_.used_blocks < header_.last_block - 1) {
      tp_assert(freeblock_stack_ != NULL, 
		"BTE_collection_ufs internal error: NULL stack pointer");
      // TODO: this is a costly operation. improve!
      size_t slen = freeblock_stack_->stream_len();
      tp_assert(slen > 0, "BTE_collection_ufs internal error: empty stack");
      if ((err = freeblock_stack_->pop(&lbn)) != BTE_ERROR_NO_ERROR)
	return err;
      bid = *lbn;
    } else {
      tp_assert(header_.last_block <= header_.total_blocks, 
		"BTE_collection_ufs internal error: last_block>total_blocks");
      if (header_.last_block == header_.total_blocks) {
	// Increase the capacity for storing blocks in the stream by
	// 16 (only by 1 the first time around to be gentle with very
	// small coll's).
	if (header_.total_blocks == 1)
	  header_.total_blocks += 2;
	else if (header_.total_blocks <= 161)
	  header_.total_blocks += 8;
	else
	  header_.total_blocks += 64;
#if USE_FTRUNCATE
	if (ftruncate(bcc_fd_, bid_to_file_offset(header_.total_blocks))) {
	  LOG_FATAL_ID("Failed to ftruncate() to the new end of file.");
	  LOG_FATAL_ID(strerror(errno));
	  return BTE_ERROR_OS_ERROR;
	}
#else
	off_t curr_off;
	char* tbuf = new char[header_.os_block_size];
	if ((curr_off = ::lseek(bcc_fd_, 0, SEEK_END)) == (off_t)-1) {
	  LOG_FATAL_ID("Failed to lseek() to the end of file.");
	  LOG_FATAL_ID(strerror(errno));
	  return BTE_ERROR_OS_ERROR;
	}
	while (curr_off < bid_to_file_offset(header_.total_blocks)) {
	  ::write(bcc_fd_, tbuf, header_.os_block_size);
	  curr_off += header_.os_block_size;
	}
	file_pointer = curr_off;
	delete [] tbuf;
#endif
      }
      bid = header_.last_block++;
    }
    return BTE_ERROR_NO_ERROR;
  }

  // Common code for all delete_block implementations. Inlined.
  BTE_err delete_block_shared(off_t bid) {
    if (bid == header_.last_block - 1) 
      header_.last_block--;
    else {
      if (freeblock_stack_ == NULL)
	create_stack();
      //tp_assert(freeblock_stack_ != NULL, 
      //	"BTE_collection_ufs internal error: NULL stack pointer");
      return freeblock_stack_->push(bid);
    }
    return BTE_ERROR_NO_ERROR;
  }

public:

  BTE_collection_base(const char *base_name, BTE_collection_type ct, 
		      size_t logical_block_factor);

  // Return the total number of used blocks.
  size_t size() const { return header_.used_blocks; }
                          	
  // Return the total number of blocks consumed by the block collection.
  size_t file_size() const { return header_.total_blocks - 1; }

  // Return the logical block size in bytes.
  size_t block_size() const { return header_.block_size; }

  // Return the logical block factor.
  size_t block_factor() const 
    { return header_.block_size / header_.os_block_size; }

  // Return the status of the collection.
  BTE_collection_status status() const { return status_; }

  // Set the persistence flag. 
  void persist(persistence p) { per_ = p; }

  const char *base_file_name() const { return base_file_name_; }

  void *user_data() { return (void *) header_.user_data; }

  // Local statistics (for this object).
  const tpie_stats_collection& stats() const { return stats_; }

  // Global statistics (for all collections).
  static const tpie_stats_collection& gstats() { return gstats_; }

  // Destructor.
  ~BTE_collection_base(); 

#if defined(__sun__) 
  static bool direct_io;
#endif
};

#endif //_BTE_COLL_BASE_H
