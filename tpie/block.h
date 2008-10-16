// Copyright (C) 2001 Octavian Procopiuc
//
// File:    ami_block.h
// Authors: Octavian Procopiuc <tavi@cs.duke.edu>
//
// Definition and implementation of the AMI_block class.
//
// $Id: ami_block.h,v 1.9 2005-01-21 17:29:26 tavi Exp $
//

#ifndef _TPIE_AMI_BLOCK_H
#define _TPIE_AMI_BLOCK_H

// Get definitions for working with Unix and Windows
#include <portability.h>

// The AMI_block_base class.
#include <block_base.h>

// The b_vector class.
#include <b_vector.h>

namespace tpie {

    namespace ami {

	template<class E, class I, class BTECOLL = bte::COLLECTION>
	class block: public block_base<BTECOLL> {
	    
	protected:
	    using block_base<BTECOLL>::bid_;
	    using block_base<BTECOLL>::dirty_;
	    using block_base<BTECOLL>::pdata_;
	    using block_base<BTECOLL>::per_;
	    using block_base<BTECOLL>::pcoll_;
	    
	public:
	    using block_base<BTECOLL>::bid;
	    
	    // The array of links.
	    b_vector<bid_t> lk;
	    
	    // The array of elements.
	    b_vector<E> el;
	    
	    typedef typename b_vector<bid_t>::iterator  lk_iterator;
	    typedef typename b_vector<E>::iterator  el_iterator;
	    
	public:
	    // Compute the capacity of the el vector statically (but you have to
	    // give the correct block size and number of links!). It can also be
	    // used to figure out how many elements would fit into a block with
	    // a given size and a given number of links.
	    static TPIE_OS_SIZE_T el_capacity(TPIE_OS_SIZE_T block_size, 
					      TPIE_OS_SIZE_T links);
	    
	    // Constructor.  Read and initialize a block with a given ID. If the
	    // ID is missing or 0, a new block is created.
	    block(collection_single<BTECOLL>* pacoll, 
		  TPIE_OS_SIZE_T links, 
		  bid_t bid = 0);
	    
	    // Get a reference to the info field.
	    I* info();
	    const I* info() const;
	};

	
////////// ***Implementation*** ///////////
	
	
	
////////////////////////////////////
////////// **block** ///////////
////////////////////////////////////
	
	template<class E, class I, class BTECOLL>
	TPIE_OS_SIZE_T block<E,I,BTECOLL>::el_capacity(TPIE_OS_SIZE_T block_size, TPIE_OS_SIZE_T links) {
	    return (TPIE_OS_SIZE_T) ((block_size - sizeof(I) - links * sizeof(bid_t)) / sizeof(E));
	}
	
	template<class E, class I, class BTECOLL>
	block<E,I,BTECOLL>::block(collection_single<BTECOLL>* pacoll, 
				  TPIE_OS_SIZE_T links, 
				  bid_t _bid):
	    block_base<BTECOLL>(pacoll, _bid), 
	    lk((bid_t*)pdata_, links),
	    el((E*) ((char*) pdata_ + links * sizeof(bid_t)), 
	       el_capacity(pcoll_->block_size(), links))
	{
	    //  No code here.
	}
	
	template<class E, class I, class BTECOLL>
	I* block<E,I,BTECOLL>::info() {
	    return (I*) (((char*) pdata_ + (lk.capacity()*sizeof(bid_t) + 
					    el.capacity()*sizeof(E))));
	}
	
	template<class E, class I, class BTECOLL>
	const I*  block<E,I,BTECOLL>::info() const {
	    return (I*) (((char*) pdata_ + (lk.capacity()*sizeof(bid_t) + 
					    el.capacity()*sizeof(E))));
	}
	
    }  //  ami namespace
    
}  //  tpie namespace

#endif // _TPIE_AMI_BLOCK_H
