// Copyright (C) 2001 Octavian Procopiuc
//
// File:    bte_coll_ufs.h
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: bte_coll_ufs.h,v 1.1 2001-05-17 19:47:37 tavi Exp $
//
// BTE_collection_ufs class definition.
//
#ifndef _BTE_COLL_UFS_H
#define _BTE_COLL_UFS_H

#include <bte_coll_base.h>


class BTE_collection_ufs: public BTE_collection_base {
public:

  // Constructors.
  BTE_collection_ufs(const char *base_file_name,
		     BTE_collection_type type = BTE_WRITE_COLLECTION,
		     size_t logical_block_factor = 1):
    BTE_collection_base(base_file_name, type, logical_block_factor) {}

  // Allocate a new block in block collection and then read that block into
  // memory, allocating and returning an appropriately initialized
  // Block. Main memory usage increases.
  BTE_err new_block(off_t &bid, void * &place);

  // Delete a previously created, currently in-memory BLOCK. This causes
  // the number of free blocks in the collection to increase by 1, the bid
  // is entered into the stdio_stack.  NOTE that it is the onus of the user
  // of this class to ensure that the bid of this placeholder is
  // correct. No check is made if the bid is an invalid or previously
  // unallocated bid, which will introduce erroneous entries in the
  // stdio_stack of free blocks. Main memory usage goes down.
  BTE_err delete_block(off_t bid, void * place);

  // Read the block with the indicated bid and allocate and initialize a
  // corresponding placeholder. NOTE once more that it is the user's onus
  // to ensure that the bid requested corresponds to a valid block and so
  // on; no checks made here to ensure that that is indeed the case. Main
  // memory usage increases.
  BTE_err get_block(off_t bid, void * &place);

  // Write a currently in-memory block. NOTE once more that it is the
  // user's onus to ensure that the bid is correct and so on; no checks
  // made here to ensure that that is indeed the case. Main memory usage
  // decreases.
  BTE_err put_block(off_t bid, void * place, char dirty = 1);

  // Synchronize the in-memory block with the on-disk block.
  BTE_err sync_block(off_t bid, void* place, char dirty = 1);
};

#endif // _BTE_COLL_UFS_H
