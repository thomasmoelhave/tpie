// Copyright (C) 2001 Octavian Procopiuc
//
// File:   ami_coll_single.h
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: ami_coll_single.h,v 1.8 2002-01-28 16:50:35 tavi Exp $
//
// AMI collection entry points implemented on top of a single BTE.
//
#ifndef _AMI_COLL_SINGLE_H
#define _AMI_COLL_SINGLE_H

// For persist type.
#include <persist.h>

// Get the tpie_stats_coll class for collection statistics.
#include <tpie_stats_coll.h>

// Get an appropriate BTE collection.
#include <bte_coll.h>

// For AMI_collection_type and AMI_collection_status.
#include <ami_coll_base.h>

// For ami_single_temp_name().
#include <ami_stream_single.h>

template<class BTECOLL=BTE_COLLECTION>
class AMI_collection_single {
public:

  // Initialize a temporary collection.
  AMI_collection_single(size_t logical_block_factor = 1);

  // Initialize a named collection.
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

  // Inquire the status.
  AMI_collection_status status() const { return status_; }
  bool is_valid() const { return status_ == AMI_COLLECTION_STATUS_VALID; }
  
  // User data to be stored in the header.
  void *user_data() { return btec_->user_data(); }

  // Destructor.
  ~AMI_collection_single() { delete btec_; }

  BTECOLL* bte() { return btec_; }

  const tpie_stats_collection& stats() const { return btec_->stats(); }

  static const tpie_stats_collection& gstats() 
    { return BTE_collection_base::gstats(); }

private:

  BTECOLL *btec_;
  AMI_collection_status status_;

  // Allow AMI_block base direct access to the BTE_COLLECTION.
  //  friend class AMI_block_base;
};

template <class BTECOLL>
AMI_collection_single<BTECOLL>::AMI_collection_single(size_t lbf) {

  char *temp_path = ami_single_temp_name("AMI");

  btec_ = new BTECOLL(temp_path, BTE_WRITE_COLLECTION, lbf);
  tp_assert(btec_ != NULL, "new failed to create a new BTE_COLLECTION.");
  btec_->persist(PERSIST_DELETE);

  if (btec_->status() == BTE_COLLECTION_STATUS_VALID)
    status_ = AMI_COLLECTION_STATUS_VALID;
  else
    status_ = AMI_COLLECTION_STATUS_INVALID;
}

template <class BTECOLL>
AMI_collection_single<BTECOLL>::AMI_collection_single(char* path_name,
		       AMI_collection_type ct = AMI_READ_WRITE_COLLECTION,
		       size_t lbf) {

  BTE_collection_type btect;

  if (ct == AMI_READ_COLLECTION)
    btect = BTE_READ_COLLECTION;
  else
    btect = BTE_WRITE_COLLECTION;
   
  btec_ = new BTECOLL(path_name, btect, lbf);
  tp_assert(btec_ != NULL, "new failed to create a new BTE_COLLECTION.");
  btec_->persist(PERSIST_PERSISTENT);

  if (btec_->status() == BTE_COLLECTION_STATUS_VALID)
    status_ = AMI_COLLECTION_STATUS_VALID;
  else
    status_ = AMI_COLLECTION_STATUS_INVALID;
}

#endif // _AMI_COLL_SINGLE_H
