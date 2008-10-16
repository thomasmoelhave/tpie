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

    ///////////////////////////////////////////////////////////////////
    /// An instance of class AMI_block<E,I> is a typed
    /// view of a logical block, which is the unit amount of data transfered
    /// between external storage and main memory. 
    
    /// The AMI_block class serves a dual purpose: (a) to provide an
    /// interface for seamless transfer of blocks between disk and main memory,
    /// and (b) to provide a structured access to the contents of the block.
    /// The first purpose is achieved through internal mechanisms, transparent
    /// to the user. When creating an instance of class AMI_block, the
    /// constructor is responsible for making the contents of the block
    /// available in main memory. When the object is deleted, the destructor is
    /// responsible for writing back the data, if necessary, and freeing the
    /// memory. Consequently, during the life of an AMI_block object, the
    /// contents of the block is available in main memory.
    /// The second purpose is achieved by partitioning the contents of the block
    /// into three fields:
    /// \par Links: 
    /// an array of pointers to other blocks, represented as
    /// block identifiers, of type AMI_bid;
    /// \par Elements: 
    /// an array of elements of parameter type E;
    /// \par Info: 
    /// an info field of parameter type I, used to store a 
    /// constant amount of administrative data;
    ///
    /// The number of elements and links that can be stored is set during
    /// construction. More specifically, the number of links is passed to the 
    /// constructor, and the number of elements fitting into one block 
    /// is computed using the following formula:
    ///
    /// number_of_elements = abs(block_size - (sizeof(I) + sizeof(AMI_bid) *
    /// number_of_links) / sizeof(E))
    ///////////////////////////////////////////////////////////////////
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
	    
	    /** The array of links. */
	    b_vector<bid_t> lk;
	    
	    /** The array of elements. */
	    b_vector<E> el;
	    
	    /** Iterator typef */
	    typedef typename b_vector<bid_t>::iterator  lk_iterator;

	    /** Iterator typef */
	    typedef typename b_vector<E>::iterator  el_iterator;
	    
	public:
      ///////////////////////////////////////////////////////////////////
  	  /// Returns the capacity (in number of elements) of the el vector, given the 
  	  /// correct block size and number of links.
      ///////////////////////////////////////////////////////////////////
	    static TPIE_OS_SIZE_T el_capacity(TPIE_OS_SIZE_T block_size, 
					      TPIE_OS_SIZE_T links);
	    
      ///////////////////////////////////////////////////////////////////
      /// Read the block with id bid from block collection
      /// *pcoll in newly allocated memory and format it using the
      /// template types and the maximum number of links links.
      /// Persistence is set to \ref PERSIST_PERSISTENT.
      ///  If the ID is missing or 0, a new block is created.
      ///////////////////////////////////////////////////////////////////
	    block(collection_single<BTECOLL>* pacoll, 
		  TPIE_OS_SIZE_T links, 
		  bid_t bid = 0);

	    ///////////////////////////////////////////////////////////////////
	    /// Returns a pointer to the info element of the block.
      ///////////////////////////////////////////////////////////////////
	    I* info();

      ///////////////////////////////////////////////////////////////////
      /// Returns a pointer to the info element of the block.
      ///////////////////////////////////////////////////////////////////
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
