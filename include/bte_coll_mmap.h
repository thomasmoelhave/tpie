//
// File:    bte_coll_mmap.h (formerly bte_coll_mmb.h)
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: bte_coll_mmap.h,v 1.7 2002-08-13 18:01:39 tavi Exp $
//
// BTE_collection_mmap class definition.
//
#ifndef _BTE_COLL_MMAP_H
#define _BTE_COLL_MMAP_H

// For header's type field (77 == 'M').
#define BTE_COLLECTION_MMAP 77

// Define write behavior.
// Allowed values:
//  0    (synchronous writes)
//  1    (asynchronous writes using MS_ASYNC - see msync(2)) [default]
//  2    (asynchronous bulk writes)
#define BTE_COLLECTION_MMAP_LAZY_WRITE 2

#include <bte_coll_base.h>

class BTE_collection_mmap: public BTE_collection_base {
public:

  // Constructor. Read and verify the header of the
  // collection. Implemented in the base class.
  BTE_collection_mmap(const char *base_file_name,
		     BTE_collection_type type = BTE_WRITE_COLLECTION,
		     size_t logical_block_factor = 1):
    BTE_collection_base(base_file_name, type, logical_block_factor) {
    header_.type = BTE_COLLECTION_MMAP;
  }

  // Allocate a new block in block collection and then map that block
  // into memory, allocating and returning an appropriately
  // initialized Block. Main memory usage increases.
  BTE_err new_block(bid_t &bid, void * &place) {
    BTE_err err;
    // Get a block id.
    if ((err = new_block_getid(bid)) != BTE_ERROR_NO_ERROR)
      return err;
    // We have a bid, so we can call the get_block routine.
    if ((err = get_block_internals(bid, place)) != BTE_ERROR_NO_ERROR)
      return err;   
    header_.used_blocks++;
    stats_.record(BLOCK_NEW);
    gstats_.record(BLOCK_NEW);
    return BTE_ERROR_NO_ERROR;
  }

  // Delete a previously created, currently mapped-in BLOCK. This causes the
  // number of free blocks in the collection to increase by 1, the bid is
  // entered into the stdio_stack.  NOTE that it is the onus of the user of
  // this class to ensure that the bid of this placeholder is correct. No
  // check is made if the bid is an invalid or previously unallocated bid,
  // which will introduce erroneous entries in the stdio_stack of free
  // blocks. Main memory usage goes down.
  BTE_err delete_block(bid_t bid, void * place) {
    BTE_err err;
    if ((err = put_block_internals(bid, place, 1)) != BTE_ERROR_NO_ERROR)  
      return err; 
    if ((err = delete_block_shared(bid)) != BTE_ERROR_NO_ERROR)
      return err;
    header_.used_blocks--;
    stats_.record(BLOCK_DELETE);
    gstats_.record(BLOCK_DELETE);
    return BTE_ERROR_NO_ERROR;
  }

  // Map in the block with the indicated bid and allocate and initialize a
  // corresponding placeholder. NOTE once more that it is the user's onus
  // to ensure that the bid requested corresponds to a valid block and so
  // on; no checks made here to ensure that that is indeed the case. Main
  // memory usage increases.
  BTE_err get_block(bid_t bid, void * &place) {
    BTE_err err;
    if ((err = get_block_internals(bid, place)) != BTE_ERROR_NO_ERROR)
      return err;
    stats_.record(BLOCK_GET);
    gstats_.record(BLOCK_GET);
    return BTE_ERROR_NO_ERROR;
  }

  // Unmap a currently mapped in block. NOTE once more that it is the user's
  // onus to ensure that the bid is correct and so on; no checks made here
  // to ensure that that is indeed the case. Main memory usage decreases.
  BTE_err put_block(bid_t bid, void * place, char dirty = 1) {
    BTE_err err;
    if ((err = put_block_internals(bid, place, dirty)) != BTE_ERROR_NO_ERROR)
      return err;
    stats_.record(BLOCK_PUT);
    gstats_.record(BLOCK_PUT);
    return BTE_ERROR_NO_ERROR;
  }

  // Synchronize the in-memory block with the on-disk block.
  BTE_err sync_block(bid_t bid, void* place, char dirty = 1);

protected:
  BTE_err get_block_internals(bid_t bid, void *&place);
  BTE_err put_block_internals(bid_t bid, void* place, char dirty);
};

#endif //_BTE_COLL_MMAP_H
