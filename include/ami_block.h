// Copyright (C) 2001 Octavian Procopiuc
//
// File:    ami_block.h
// Authors: Octavian Procopiuc <tavi@cs.duke.edu>
//
// Definition and implementation of the AMI_block class.
//
// $Id: ami_block.h,v 1.1 2001-05-17 19:49:42 tavi Exp $
//

#ifndef _AMI_BLOCK_H
#define _AMI_BLOCK_H

// The AMI_block_base class.
#include <ami_block_base.h>

// The b_vector class.
#include <b_vector.h>
 
template<class E, class I>
class AMI_block: public AMI_block_base {

public:
  
  // The array of links.
  b_vector<AMI_block_id> lk;

  // The array of elements.
  b_vector<E> el;

  typedef b_vector<AMI_block_id>::iterator  lk_iterator;
  typedef b_vector<E>::iterator  el_iterator;

public:

  // Constructor.  Read and initialize a block with a given ID. If the
  // ID is missing or 0, a new block is created.
  AMI_block(AMI_COLLECTION* pacoll, size_t links, AMI_block_id bid = 0);

  // Get a reference to the info field.
  I* info();
  const I* info() const;
};


////////// ***Implementation*** ///////////



////////////////////////////////
////////// **AMI_block** ///////////
////////////////////////////////

template<class E, class I>
AMI_block<E,I>::AMI_block(AMI_COLLECTION* pacoll, 
		  size_t links, AMI_block_id bid):
  AMI_block_base(pacoll, bid), lk((AMI_block_id*)pdata_, links),
  el((E*) ((char*) pdata_ + links * sizeof(AMI_block_id)),
     (size_t) ((pcoll_->block_size()-sizeof(I)-links*sizeof(AMI_block_id))/sizeof(E)))
{
}

template<class E, class I>
I* AMI_block<E,I>::info() {
  return (I*) (((char*) pdata_ + (lk.capacity()*sizeof(AMI_block_id) + 
				  el.capacity()*sizeof(E))));
}

template<class E, class I>
const I*  AMI_block<E,I>::info() const {
  return (I*) (((char*) pdata_ + (lk.capacity()*sizeof(AMI_block_id) + 
				  el.capacity()*sizeof(E))));
}

#endif // _AMI_BLOCK_H
