// Copyright (C) 2001 Octavian Procopiuc
//
// File:   ami_coll_single.h
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: ami_coll_single.h,v 1.1 2001-05-17 19:33:10 tavi Exp $
//
// AMI collection entry points implemented on top of a single BTE.
//
#ifndef _AMI_COLL_SINGLE_H
#define _AMI_COLL_SINGLE_H

// For persist type.
#include <persist.h>

// For tp_assert().
#include <tpie_assert.h>

// Get an appropriate BTE collection.
#include <bte_coll.h>

// For AMI_collection_type and AMI_collection_status.
#include <ami_coll_base.h>

// For ami_single_temp_name().
#include <ami_single.h>

class AMI_collection_single {
public:

  // A temporary collection.
  AMI_collection_single(size_t logical_block_factor = 1);

  AMI_collection_single(char* path_name,
			AMI_collection_type ct = AMI_READ_WRITE_COLLECTION,
			size_t logical_block_factor = 1);

  // Return the total number of used blocks.
  size_t size() const { return btec_->size(); }

  // Return the logical block size in bytes.
  size_t block_size() const { return btec_->block_size(); }

  // Return the logical block factor.
  size_t block_factor() const { return btec_->block_factor(); }

  // Set the persistence flag. 
  void persist(persistence p) { btec_->persist(p); }

  AMI_collection_status status() const { return status_; }

  // User data to be stored in the header.
  void *user_data() { return btec_->user_data(); }

  // Destructor.
  ~AMI_collection_single() { delete btec_; }

private:

  BTE_COLLECTION *btec_;
  AMI_collection_status status_;

  BTE_COLLECTION* bte() { return btec_; }

  friend class AMI_block_base;
};



#endif // _AMI_COLL_SINGLE_H
