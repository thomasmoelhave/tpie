// Copyright (C) 2001 Octavian Procopiuc
//
// File:    ami_block_base.h
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: ami_block_base.h,v 1.1 2001-05-17 19:45:02 tavi Exp $
//
// Definition of the AMI_block_base class.
//

#ifndef _AMI_BLOCK_BASE_H
#define _AMI_BLOCK_BASE_H

#include <ami_coll.h>

// The Block_stats class.
#include <ami_block_stats.h>

//
// Block status type
//
enum AMI_block_status {
  AMI_BLOCK_STATUS_VALID = 0,
  AMI_BLOCK_STATUS_INVALID = 1,
};

class AMI_block_base {
protected:

  // Pointer to the block collection.
  BTE_COLLECTION * pcoll_;

  // Unique ID. Represents the offset of the block in the blocks file.
  AMI_block_id bid_;

  // Dirty bit. If set, the block needs to be written back.
  char dirty_;

  // Pointer to the actual data.
  void * pdata_;

  // Persistence flag.
  persistence per_;

  // Statistics object, one for all blocks.
  static AMI_block_stats stats_;

  friend AMI_block_stats& AMI_block_statistics();

public:

  // Constructor.
  // Read and initialize a block with a given ID.
  // When bid is missing or 0, a new block is created.
  AMI_block_base(AMI_COLLECTION* pacoll, AMI_block_id bid = 0)
    : bid_(bid), dirty_(0), per_(PERSIST_PERSISTENT) {
    pcoll_ = pacoll->bte();
    if (bid != 0) {
      // Get an existing block from disk.
      if (pcoll_->get_block(bid_, pdata_) != BTE_ERROR_NO_ERROR)
	pdata_ = NULL;
      else
	stats_.record(BLOCK_READ);
    } else {
      // Create a new block in the collection.
      if (pcoll_->new_block(bid_, pdata_) != BTE_ERROR_NO_ERROR)
	pdata_ = NULL;
      else
	stats_.record(BLOCK_CREATE);
    }
  }

  AMI_err sync() {
    if (pcoll_->sync_block(bid_, pdata_) != BTE_ERROR_NO_ERROR)
      return AMI_ERROR_BTE_ERROR;
    else
      return AMI_ERROR_NO_ERROR;
  }

  // Get the block id.
  AMI_block_id bid() const { return bid_; }

  // Get a reference to the dirty bit.
  char& dirty() { return dirty_; };
  char dirty() const { return dirty_; }

  // Copy block rhs into this block.
  AMI_block_base& operator=(const AMI_block_base& rhs) { 
    if (pcoll_ == rhs.pcoll_) {
      memcpy(pdata_, rhs.pdata_, pcoll_->block_size());
      dirty_ = 1;
    } else 
      pdata_ = NULL;
    return *this; 
  }

  // Get the block's status.
  AMI_block_status status() const { 
    return (pdata_ == NULL) ? 
      AMI_BLOCK_STATUS_INVALID: AMI_BLOCK_STATUS_VALID; 
  }

  void persist(persistence per) { per_ = per; }

  persistence persist() const { return per_; }

  size_t block_size() const { return pcoll_->block_size(); }

  // Destructor.
  ~AMI_block_base() {
    // Check first the status of the collection. 
    if (pdata_ != NULL)
      if (per_ == PERSIST_PERSISTENT) {
	// Write back the block.
	if (pcoll_->put_block(bid_, pdata_) == BTE_ERROR_NO_ERROR)
	  stats_.record(BLOCK_WRITE);
      } else {
	// Delete the block from the collection.
	if (pcoll_->delete_block(bid_, pdata_) == BTE_ERROR_NO_ERROR)
	  stats_.record(BLOCK_DELETE);
      }
  }
};

// *AMI_block_base::stats_*
AMI_block_stats AMI_block_base::stats_;


AMI_block_stats& AMI_block_statistics() {
  return AMI_block_base::stats_;
}

#endif _AMI_BLOCK_BASE_H
