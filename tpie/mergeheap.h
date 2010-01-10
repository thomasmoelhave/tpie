// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

///////////////////////////////////////////////////////////////////////////
/// \file mergeheap.h
/// This file contains several merge heap templates. 
/// Originally written by Rakesh Barve.  
///
/// The heap is basically the heap from CLR except that there is
/// provision to exploit the fact that when you are merging you know
/// you will be inserting a new element whenever you are doing a
/// delete_min.
///
/// Modified by David Hutchinson 2000 03 02
///
///     - main purpose of the mods is to allow the merge heap to be
///     part of a sort management object. The sort management object
///     contains several procedures and data structures needed for
///     sorting, but the precise versions of these procedures and data
///     structures depend on the sorting approach used (this permits
///     parameterization of the sorting procedure via the sort
///     management object, and avoids having multiple versions of large
///     pieces of code that are highly redundant and difficult to
///     maintain).
///
///     - move initialization from constructor to an explicit
///     "initialize" member function
///
///     - add a "comparison object" version of the merge heap
///     object. This allows a comparison object with a "compare" member
///     function to be specified for comparing keys. "Comparison
///     operator" and "comparison function" versions previously
///     existed.
///
///     - add a set of two (comparison object, operator)
///     versions of the merge heap that maintain pointers to the
///     current records at the head of the streams being merged. The
///     previous versions kept the entire corresponding record in the heap.
/// \deprecated 
///  Additional to the still valid functionality in this file, 
///  earlier TPIE versions allowed a heap that uses C-style
///  comparison functions. However, comparison functions cannot be
///  inlined, so each comparison requires one function call. Given that the
///  comparison operator < and comparison object classes can be inlined and
///  have better performance while providing the exact same functionality,
///  comparison functions have been removed from TPIE. If you can provide us
///  with a compelling argument on why they should be in here, we may consider
///  adding them again, but you must demonstrate that comparision functions
///  can outperform other methods in at least some cases or give an example
///  were it is impossible to use a comparison operator or comparison object
///////////////////////////////////////////////////////////////////////////

#ifndef _MERGE_HEAP_H
#define _MERGE_HEAP_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// Macros for left and right.
#define Left(i)   2*(i)
#define Right(i)  2*(i)+1
#define Parent(i) (i)/2

namespace tpie {

    namespace ami {
	
  ///////////////////////////////////////////////////////////////////////////
  /// This is a heap element. Encapsulates the key, along with
  /// the label run_id indicating the run the key originates from.
  ///////////////////////////////////////////////////////////////////////////
	template<class KEY>
	class heap_element {
	public:
	    heap_element() : key(), run_id(0) {}
	    KEY            key;
	    memory_size_type run_id;
	};
	
    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {
  ///////////////////////////////////////////////////////////////////////////
  /// This is a record pointer element. Encapsulates the record pointer,
  /// along with the label run_id indicating the run the record
  /// originates from.
  ///////////////////////////////////////////////////////////////////////////
	template<class REC>
	class heap_ptr {
	public:
	    heap_ptr() : recptr(NULL), run_id(0) {};
	    heap_ptr(const heap_ptr<REC>& other) {
		*this = other;
	    }
	    heap_ptr<REC>& operator=(const heap_ptr<REC>& other) {
		if (this != &other) {
		    recptr = other.recptr;
	    run_id = other.run_id;
		}
	    }
	    
	    ~heap_ptr(){};
	    REC            *recptr;
	    memory_size_type run_id;
	};
	

    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {
	
  ///////////////////////////////////////////////////////////////////////////
  /// A record pointer heap base class - also serves as the full
  /// implementation for objects with a < comparison operator.
  ///////////////////////////////////////////////////////////////////////////
	template<class REC>
	class merge_heap_ptr_op{
	    
	protected:
	    
	    heap_ptr<REC> *Heaparray;
	    memory_size_type  Heapsize;
	    memory_size_type  maxHeapsize;
	    
	    inline void Exchange(memory_size_type i, memory_size_type j); 
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// These functions will typically be overridden by subclasses/
	    ///////////////////////////////////////////////////////////////////////////
	    inline memory_size_type get_smallest(memory_size_type i);
	    inline void Heapify(memory_size_type i);
	    
	public:
	    
  	  ///////////////////////////////////////////////////////////////////////////
  	  /// Constructor/
  	  ///////////////////////////////////////////////////////////////////////////
	    merge_heap_ptr_op() : Heaparray(NULL), Heapsize(0), maxHeapsize(0) {};
	    
      ///////////////////////////////////////////////////////////////////////////
      /// Copy Constructor.
      ///////////////////////////////////////////////////////////////////////////
	    merge_heap_ptr_op(const merge_heap_ptr_op<REC>& other) {
	*this = other;
	    }
	    
      ///////////////////////////////////////////////////////////////////////////
      /// Destructor.
      ///////////////////////////////////////////////////////////////////////////
	    ~merge_heap_ptr_op() { 
		//Cleanup if someone forgot de-allocate
		if(Heaparray != NULL){
		    delete [] Heaparray;
		}
	    }

      ///////////////////////////////////////////////////////////////////////////
      /// Assignment operator.
      ///////////////////////////////////////////////////////////////////////////
	    merge_heap_ptr_op& operator=(const merge_heap_ptr_op<REC>& other) {
		if (this != &other) {
		    Heapsize    = other.Heapsize;
		    maxHeapsize = other.maxHeapsize;
		    
		    if (Heaparray != NULL) {
			delete[] Heaparray;
		    }
		    
		    Heaparray = new heap_ptr<REC>[maxHeapsize+1];
		    memcpy(Heaparray, 
			   other.Heaparray, 
			   maxHeapsize * sizeof(heap_ptr<REC>));
		}
		return *this;
	    }
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Reports the size of Heap (number of elements).
	    ///////////////////////////////////////////////////////////////////////////
	    memory_size_type sizeofheap(void) {
		return Heapsize;
	    }; 

	    ///////////////////////////////////////////////////////////////////////////
	    /// Returns the run with the minimum key.
	    ///////////////////////////////////////////////////////////////////////////
	    inline memory_size_type get_min_run_id(void) {
		return Heaparray[1].run_id;
	    };
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Allocates space for the heap.
	    ///////////////////////////////////////////////////////////////////////////
	    void allocate   (memory_size_type size);

	    ///////////////////////////////////////////////////////////////////////////
	     /// Copies an (initial) element into the heap array.
	     ///////////////////////////////////////////////////////////////////////////
	    void insert     (REC *ptr, memory_size_type run_id);
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Extracts minimum element from heap array.
	    /// If you follow this with an immediate insert, consider using
	    /// delete_min_and_insert().
	    ///////////////////////////////////////////////////////////////////////////
	    void extract_min(REC& el, memory_size_type& run_id);

	    ///////////////////////////////////////////////////////////////////////////
	    /// Deallocates the space used by the heap.
	    ///////////////////////////////////////////////////////////////////////////
	    void deallocate (void);
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Heapifies an initial array of elements;
	    /// typically overridden in sub class. 
	    ///////////////////////////////////////////////////////////////////////////
	    void initialize (void);
	    
	    ///////////////////////////////////////////////////////////////////////////
	    // Deletes the current minimum and inserts the new item from the same
	    // source / run.
	    ///////////////////////////////////////////////////////////////////////////
	    inline void delete_min_and_insert(REC *nextelement_same_run);
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Returns the main memory space usage per item.
	    ///////////////////////////////////////////////////////////////////////////
	    inline memory_size_type space_per_item(void) { return sizeof(heap_ptr<REC>); }
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Returns the fixed main memory space overhead, regardless of item count.
	    ///////////////////////////////////////////////////////////////////////////
	    inline memory_size_type space_overhead(void) { 
		// One extra array item is defined to make heap indexing easier
		return sizeof(heap_ptr<REC>)+MM_manager.space_overhead();
	    }
	    
	};
	
	template<class REC>
	inline void merge_heap_ptr_op<REC>::Exchange(memory_size_type i,
						     memory_size_type j)
	{
	    REC* tmpptr;
	    memory_size_type tmpid;
	    tmpptr = Heaparray[i].recptr;
	    tmpid = Heaparray[i].run_id;   
	    Heaparray[i].recptr = Heaparray[j].recptr;
	    Heaparray[i].run_id = Heaparray[j].run_id;
	    Heaparray[j].recptr = tmpptr;
	    Heaparray[j].run_id = tmpid;
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// Returns the index of the smallest element out of
	/// \p i, the left child of \p i, and the right child of \p i.
	///////////////////////////////////////////////////////////////////////////
	template<class REC>
	inline memory_size_type merge_heap_ptr_op<REC>::get_smallest(
	    memory_size_type i)
	{
	    memory_size_type l,r, smallest;
 
	    l = Left(i);
	    r = Right(i);
	    
	    smallest = ((l <= Heapsize) && 
			(*Heaparray[l].recptr < *Heaparray[i].recptr)) ? l : i;
	    
	    smallest = ((r <= Heapsize) && 
			(*Heaparray[r].recptr < *Heaparray[smallest].recptr))? r : smallest;
	    
	    return smallest;
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// This is the primary heapify function; note that we have unfolded the 
	/// recursion.
	///////////////////////////////////////////////////////////////////////////
	template<class REC>
	inline void merge_heap_ptr_op<REC>::Heapify(memory_size_type i) {
	    
	    memory_size_type smallest = get_smallest(i);
	    
	    while (smallest != i) {
		this->Exchange(i,smallest);
		i = smallest;
		smallest = get_smallest(i);
	    }
	}
	
	template<class REC>
	inline void merge_heap_ptr_op<REC>::delete_min_and_insert
	(REC *nextelement_same_run)
	{ 
	    if (nextelement_same_run == NULL) {
		Heaparray[1].recptr = Heaparray[Heapsize].recptr;
		Heaparray[1].run_id = Heaparray[Heapsize].run_id;
		Heapsize--;
	    } else { 
		Heaparray[1].recptr = nextelement_same_run;
	    }
	    Heapify(1);
	}
	
	template<class REC>
	inline void merge_heap_ptr_op<REC>::extract_min(REC& el, memory_size_type& run_id)
	{
	    el=*(Heaparray[1].recptr);
	    run_id=Heaparray[1].run_id;
	    Heaparray[1]=Heaparray[Heapsize--];
	    Heapify(1);
	}
	
	template<class REC>
	inline void merge_heap_ptr_op<REC>::allocate ( memory_size_type size ) {
	    Heaparray = new heap_ptr<REC> [size+1];
	    Heapsize  = 0;
	    maxHeapsize = size;
	}
	
	template<class REC>
	inline void merge_heap_ptr_op<REC>::insert (REC *ptr, memory_size_type run_id)
	{
	    Heaparray[Heapsize+1].recptr    = ptr;
	    Heaparray[Heapsize+1].run_id = run_id;
	    Heapsize++;
	}
	
	template<class REC>
  inline void merge_heap_ptr_op<REC>::deallocate () {
	    if (Heaparray){
		delete [] Heaparray; 
		Heaparray=NULL;
	    }
	    Heapsize    = 0;
	    maxHeapsize = 0;
	}
	
	template<class REC>
	void merge_heap_ptr_op<REC>::initialize () {
	    for ( memory_size_type i = Heapsize/2; i >= 1; i--){ Heapify(i); }
	}
	

    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {

  ///////////////////////////////////////////////////////////////////////////
  /// A record pointer heap that uses a comparison object
  ///////////////////////////////////////////////////////////////////////////
	template<class REC, class CMPR>
	class merge_heap_ptr_obj: public merge_heap_ptr_op<REC>{
	    
	protected: 
	    
	    using merge_heap_ptr_op<REC>::Heapsize;
	    using merge_heap_ptr_op<REC>::Heaparray;
	    using merge_heap_ptr_op<REC>::maxHeapsize;
	    CMPR* cmp;
	    
	    inline memory_size_type get_smallest(memory_size_type i);
	    inline void Heapify(memory_size_type i);
	    
	public:
	    using merge_heap_ptr_op<REC>::sizeofheap;
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Constructor; initializes a pointer to the user's comparison object.
	    /// The object may contain dynamic data although the 'compare' method is const
	    /// and therefore inline'able.
	    ///////////////////////////////////////////////////////////////////////////
	    merge_heap_ptr_obj ( CMPR *cmptr ) : cmp(cmptr) {};
	    ~merge_heap_ptr_obj(){};
	    
      ///////////////////////////////////////////////////////////////////////////
      /// Extracts minimum element from heap array.
      /// If you follow this with an immediate insert, consider using
      /// delete_min_and_insert().
      ///////////////////////////////////////////////////////////////////////////
	    void extract_min(REC& el, memory_size_type& run_id);
	    
	    void initialize (void);
  
	    ///////////////////////////////////////////////////////////////////////////
	    /// Deletes the current minimum and inserts the new item from the same
	    /// source / run.
	    ///////////////////////////////////////////////////////////////////////////
	    inline void delete_min_and_insert(REC *nextelement_same_run);

	private:
	    // Prohibit these
	    merge_heap_ptr_obj(const merge_heap_ptr_obj<REC,CMPR>& other);
	    merge_heap_ptr_obj<REC,CMPR>& operator=(const merge_heap_ptr_obj<REC,CMPR>& other);
	};
	
	///////////////////////////////////////////////////////////////////////////
	/// Returns the index of the smallest element out of
	/// \p i, the left child of \p i, and the right child of \p i.
	///////////////////////////////////////////////////////////////////////////
	template<class REC, class CMPR>
	inline memory_size_type merge_heap_ptr_obj<REC,CMPR>::get_smallest(
	    memory_size_type i)
	{
	    memory_size_type l,r, smallest;
	    
	    l = Left(i);
	    r = Right(i);
	    
	    smallest = ((l <= Heapsize) && 
			(cmp->compare(*Heaparray[l].recptr,*Heaparray[i].recptr)< 0)) ? l : i;
	    
	    smallest = ((r <= Heapsize) && 
			(cmp->compare(*Heaparray[r].recptr,*Heaparray[smallest].recptr)<0))?
		r : smallest;
	    
	    return smallest;
	}
	
	template<class REC, class CMPR>
	inline void merge_heap_ptr_obj<REC, CMPR>::Heapify(memory_size_type i) {
	    memory_size_type smallest = get_smallest(i);
	    while (smallest != i) {
		this->Exchange(i,smallest);
		i = smallest;
		smallest = get_smallest(i);
	    }
	}
	
	template<class REC, class CMPR>
	inline void merge_heap_ptr_obj<REC, CMPR>::delete_min_and_insert
	(REC *nextelement_same_run)
	{ 
	    if (nextelement_same_run == NULL) {
		Heaparray[1].recptr = Heaparray[Heapsize].recptr;
		Heaparray[1].run_id = Heaparray[Heapsize].run_id;
		Heapsize--;
	    } else { 
		Heaparray[1].recptr = nextelement_same_run;
	    }
	    Heapify(1);
	}
	
	///////////////////////////////////////////////////////////////////////////
  /// Extracts the minimum element from heap array.
  /// If you follow this with an immediate insert, consider using
  /// delete_min_and_insert().
	///////////////////////////////////////////////////////////////////////////
	template<class REC, class CMPR>
	inline void merge_heap_ptr_obj<REC, CMPR>::extract_min
	(REC& el, memory_size_type& run_id)
	{
	    el=*(Heaparray[1].recptr);
	    run_id=Heaparray[1].run_id;
	    Heaparray[1]=Heaparray[Heapsize--];
	    Heapify(1);
	}
	
	
	template<class REC, class CMPR>
	void merge_heap_ptr_obj<REC, CMPR>::initialize () {
	    for ( memory_size_type i = Heapsize/2; i >= 1; i--){ Heapify(i); }
	}


    }   // ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {
	
  ///////////////////////////////////////////////////////////////////////////
  /// A merge heap object base class - also serves as the full
  /// implementation for objects with a < comparison operator
  ///////////////////////////////////////////////////////////////////////////
	template<class REC>
	class merge_heap_op{
	    
	protected:
	    
	    heap_element<REC> *Heaparray;
	    memory_size_type  Heapsize;
	    memory_size_type  maxHeapsize;
	    
	    inline void Exchange(memory_size_type i, memory_size_type j); 
	    inline void Heapify(memory_size_type i);
	    ///////////////////////////////////////////////////////////////////////////
	    /// This function will typically be overridden by subclasses.
	    ///////////////////////////////////////////////////////////////////////////
	    inline memory_size_type get_smallest(memory_size_type i);
	    
	public:
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Constructor.
	    ///////////////////////////////////////////////////////////////////////////
	    merge_heap_op() : Heaparray(NULL), Heapsize(0), maxHeapsize(0) {
		// Do nothing.
	    };
	    
      ///////////////////////////////////////////////////////////////////////////
      /// Copy Constructor.
      ///////////////////////////////////////////////////////////////////////////
	    merge_heap_op(const merge_heap_op& other) {
		*this = other;
	    }
	    
      ///////////////////////////////////////////////////////////////////////////
      /// Destructor.
      ///////////////////////////////////////////////////////////////////////////
	    ~merge_heap_op() { 
		//Cleanup if someone forgot de-allocate
		if(Heaparray != NULL){
		    delete [] Heaparray;
		}
	    }
	    
      ///////////////////////////////////////////////////////////////////////////
      /// Assignment operator.
      ///////////////////////////////////////////////////////////////////////////
	    merge_heap_op& operator=(const merge_heap_op& other) {
		if (this != &other) {
		    Heapsize    = other.Heapsize;
		    maxHeapsize = other.maxHeapsize;
		    
		    if (Heaparray != NULL) {
			delete[] Heaparray;
		    }
		    
		    Heaparray = new heap_element<REC>[maxHeapsize+1];
		    memcpy(Heaparray, 
			   other.Heaparray, 
			   maxHeapsize * sizeof(heap_element<REC>));
		}
		return *this;
	    }
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Reports the  size of Heap (number of elements).
	    ///////////////////////////////////////////////////////////////////////////
	    memory_size_type sizeofheap(void) {
		return Heapsize;
	    }; 
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Returns the run with the minimum key.
	    ///////////////////////////////////////////////////////////////////////////
	    inline memory_size_type get_min_run_id(void) {
		return Heaparray[1].run_id;
	    };
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Allocates space for the heap.
	    ///////////////////////////////////////////////////////////////////////////
	    void allocate   (memory_size_type size);

	    ///////////////////////////////////////////////////////////////////////////
      /// Copies an (initial) element into the heap array/
      ///////////////////////////////////////////////////////////////////////////
      void insert     (REC *ptr, memory_size_type run_id);

	    ///////////////////////////////////////////////////////////////////////////
      /// Extracts minimum element from heap array.
      /// If you follow this with an immediate insert, consider using
      /// delete_min_and_insert().
      ///////////////////////////////////////////////////////////////////////////
	    void extract_min(REC& el, memory_size_type& run_id);

	    ///////////////////////////////////////////////////////////////////////////
	    /// Deallocates the space used by the heap.
	    ///////////////////////////////////////////////////////////////////////////
	    void deallocate (void);
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Heapifies an initial array of elements.
	    ///////////////////////////////////////////////////////////////////////////
	    void initialize (void);
	    
	    ///////////////////////////////////////////////////////////////////////////
	    // Deletes the current minimum and inserts the new item from the same
	    // source / run.
	    ///////////////////////////////////////////////////////////////////////////
	    inline void delete_min_and_insert(REC *nextelement_same_run);
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Returns the  main memory space usage per item
	    ///////////////////////////////////////////////////////////////////////////
	    inline memory_size_type space_per_item(void) {
		return sizeof(heap_element<REC>);
	    }
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Returns the  fixed main memory space overhead, regardless of item count.
	    ///////////////////////////////////////////////////////////////////////////
	    inline memory_size_type space_overhead(void) { 
		// One extra array item is defined to make heap indexing easier
		return sizeof(heap_element<REC>)+MM_manager.space_overhead();
	    }
	    
	};
	
	template<class REC>
	inline void merge_heap_op<REC>::Exchange(memory_size_type i,
						 memory_size_type j)
	{
	    REC tmpkey;
	    memory_size_type tmpid;
	    tmpkey = Heaparray[i].key;
	    tmpid = Heaparray[i].run_id;   
	    Heaparray[i].key = Heaparray[j].key;
	    Heaparray[i].run_id = Heaparray[j].run_id;
	    Heaparray[j].key = tmpkey;
	    Heaparray[j].run_id = tmpid;
	}
	
//Returns the index of the smallest element out of
//i, the left child of i, and the right child of i
	template<class REC>
	inline memory_size_type merge_heap_op<REC>::get_smallest(
	    memory_size_type i)
	{
	    memory_size_type l,r, smallest;
	    
	    l = Left(i);
	    r = Right(i);

	    smallest = ((l <= Heapsize) && 
			(Heaparray[l].key < Heaparray[i].key)) ? l : i;
	    
	    smallest = ((r <= Heapsize) && 
			(Heaparray[r].key < Heaparray[smallest].key))? r : smallest;
	    
	    return smallest;
	}
	
// This is the primary function; note that we have unfolded the 
// recursion.
	template<class REC>
	inline void merge_heap_op<REC>::Heapify(memory_size_type i) {
	    
	    memory_size_type smallest = get_smallest(i);
	    
	    while (smallest != i) {
		this->Exchange(i,smallest);
		i = smallest;
		smallest = get_smallest(i);
	    }
	}
	
	template<class REC>
	inline void merge_heap_op<REC>::delete_min_and_insert
	(REC *nextelement_same_run)
	{ 
	    if (nextelement_same_run == NULL) {
		Heaparray[1].key = Heaparray[Heapsize].key;
		Heaparray[1].run_id = Heaparray[Heapsize].run_id;
		Heapsize--;
	    } else { 
		Heaparray[1].key = *nextelement_same_run;
	    }
	    Heapify(1);
	}
	
// Extract minimum element from heap array
// If you follow this with an immediate insert, consider using
// delete_min_and_insert
	template<class REC>
	inline void merge_heap_op<REC>::extract_min(REC& el, memory_size_type& run_id)
	{
	    el=Heaparray[1].key;
	    run_id=Heaparray[1].run_id;
	    Heaparray[1]=Heaparray[Heapsize--];
	    Heapify(1);
	}
	
// Allocate space for the heap
	template<class REC>
	inline void merge_heap_op<REC>::allocate ( memory_size_type size ) {
	    Heaparray = new heap_element<REC> [size+1];
	    Heapsize  = 0;
	    maxHeapsize = size;
	}
	
// Copy an (initial) element into the heap array
	template<class REC>
	inline void merge_heap_op<REC>::insert (REC *ptr, memory_size_type run_id)
	{
	    Heaparray[Heapsize+1].key    = *ptr;
	    Heaparray[Heapsize+1].run_id = run_id;
	    Heapsize++;
	}
	
// Deallocate the space used by the heap
	template<class REC>
	inline void merge_heap_op<REC>::deallocate () {
	    if (Heaparray){
		delete [] Heaparray; 
		Heaparray=NULL;
	    }
	    Heapsize    = 0;
	    maxHeapsize = 0;
	};
	
	template<class REC>
	void merge_heap_op<REC>::initialize () {
	    for ( memory_size_type i = Heapsize/2; i >= 1; i--){ Heapify(i); }
	}
	       
    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {
	
// ********************************************************************
// * A merge heap that uses a comparison object                       *
// ********************************************************************
	
	template<class REC, class CMPR>
	class merge_heap_obj: public merge_heap_op<REC>{
	    
	protected:   
	    using merge_heap_op<REC>::Heapsize;
	    using merge_heap_op<REC>::Heaparray;
	    using merge_heap_op<REC>::maxHeapsize;
	    CMPR* cmp;
	    
	    inline memory_size_type get_smallest(memory_size_type i);
	    
	    inline void Heapify(memory_size_type i);
	    
	public:
	    using merge_heap_op<REC>::sizeofheap;
	    
	    // Constructor initializes a pointer to the user's comparison object
	    // The object may contain dynamic data although the 'compare' method is const
	    // and therefore inline'able.
	    merge_heap_obj ( CMPR *cmptr ) : cmp(cmptr) {};
	    ~merge_heap_obj(){};
	    
	    void extract_min(REC& el, memory_size_type& run_id);
	    
	    // heapify's an initial array of elements
	    void initialize (void);
	    
	    // Delete the current minimum and insert the new item from the same
	    // source / run.
	    inline void delete_min_and_insert(REC *nextelement_same_run);
	    
	private:
	    // Prohibit these
	    merge_heap_obj(const merge_heap_obj<REC,CMPR>& other);
	    merge_heap_obj<REC,CMPR>& operator=(const merge_heap_obj<REC,CMPR>& other);
	};
	
//Returns the index of the smallest element out of
//i, the left child of i, and the right child of i
	template<class REC, class CMPR>
	inline memory_size_type merge_heap_obj<REC,CMPR>::get_smallest(
	    memory_size_type i)
	{
	    memory_size_type l,r, smallest;
	    
	    l = Left(i);
	    r = Right(i);
	    
	    smallest = ((l <= Heapsize) &&
			(cmp->compare(Heaparray[l].key,Heaparray[i].key)< 0)) ? l : i;
	    
	    smallest = ((r <= Heapsize) && 
			(cmp->compare(Heaparray[r].key,Heaparray[smallest].key)<0))?
		r : smallest;
	    
  return smallest;
	}
	
	template<class REC, class CMPR>
	inline void merge_heap_obj<REC, CMPR>::Heapify(memory_size_type i) {
	    memory_size_type smallest = get_smallest(i);
	    while (smallest != i) {
		this->Exchange(i,smallest);
		i = smallest;
      smallest = get_smallest(i);
	    }
	}
	
	template<class REC, class CMPR>
	inline void merge_heap_obj<REC, CMPR>::delete_min_and_insert
	(REC *nextelement_same_run)
	{ 
	    if (nextelement_same_run == NULL) {
		Heaparray[1].key = Heaparray[Heapsize].key;
		Heaparray[1].run_id = Heaparray[Heapsize].run_id;
		Heapsize--;
	    } else { 
		Heaparray[1].key = *nextelement_same_run;
	    }
	    Heapify(1);
	}
	
// Extract minimum element from heap array
// If you follow this with an immediate insert, consider using
// delete_min_and_insert
	template<class REC, class CMPR>
	inline void merge_heap_obj<REC, CMPR>::extract_min
	(REC& el, memory_size_type& run_id)
	{
	    el=Heaparray[1].key;
	    run_id=Heaparray[1].run_id;
	    Heaparray[1]=Heaparray[Heapsize--];
	    Heapify(1);
	}
	

	template<class REC, class CMPR>
	void merge_heap_obj<REC, CMPR>::initialize () {
	    for ( memory_size_type i = Heapsize/2; i >= 1; i--){ Heapify(i); }
	}
	
    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {

// ********************************************************************
// * A merge heap key-object base class                               *
// * Also serves as a full impelementation of a                       *
// * key-merge heap that uses a comparison operator <                 *
// ********************************************************************
	
// The merge_heap_kop object maintains only the keys in its heap,
// and uses the member function "copy" of the user-provided class CMPR
// to copy these keys from each record.
	
	template<class REC, class KEY, class CMPR>
	class merge_heap_kop{
	    
	protected:
	    
	    heap_element<KEY> *Heaparray;
	    memory_size_type  Heapsize;
	    memory_size_type  maxHeapsize;
	    inline void Exchange(memory_size_type i, memory_size_type j);
	    inline void Heapify(memory_size_type i);
	    //This function will typically be overridden by subclasses
	    inline memory_size_type get_smallest(memory_size_type i);
	    CMPR *UsrObject;
	    
	public:
	    
	    // Constructor/Destructor 
	    merge_heap_kop( CMPR* cmpptr) : 
		Heaparray(NULL), Heapsize(0), maxHeapsize(0), UsrObject(cmpptr) {};
	    ~merge_heap_kop() { 
		//Cleanup if someone forgot de-allocate
		//(abd) This seems to cause double free errors, but I don't know why
		//This was just a safeguard anyways, turn off for now
		//if(Heaparray != NULL){delete [] Heaparray;}
	    }
	    
	    // Report size of Heap (number of elements)
	    memory_size_type sizeofheap(void) {return Heapsize;}; 
	    
	    // Return the run with the minimum key.
	    inline memory_size_type get_min_run_id(void) {return Heaparray[1].run_id;};
	    
	    void allocate   (memory_size_type size);
	    void insert     (REC *ptr, memory_size_type run_id);
	    void deallocate ();
	    
	    // Delete the current minimum and insert the new item from the same
	    // source / run.
	    inline void delete_min_and_insert(REC *nextelement_same_run);
	    
	    // Return main memory space usage per item
	    inline memory_size_type space_per_item(void) {
		return sizeof(heap_element<REC>);
	    }
	    
	    // Return fixed main memory space overhead, regardless of item count
	    inline memory_size_type space_overhead(void) { 
		// One extra array item is defined to make heap indexing easier
		return sizeof(heap_ptr<REC>)+MM_manager.space_overhead();
	    }
	    
	    // heapify's an initial array of elements
	    void initialize (void);
	    
	private:
	    // Prohibit these
	    merge_heap_kop(const merge_heap_kop<REC,KEY,CMPR>& other);
	    merge_heap_kop<REC,KEY,CMPR>& operator=(const merge_heap_kop<REC,KEY,CMPR>& other);
	};
	
	template<class REC, class KEY, class CMPR>
	inline void merge_heap_kop<REC,KEY,CMPR>::Exchange(memory_size_type i,
							   memory_size_type j)
	{
	    KEY tmpkey;
	    memory_size_type tmpid;
	    tmpkey = Heaparray[i].key;
	    tmpid = Heaparray[i].run_id;   
	    Heaparray[i].key = Heaparray[j].key;
	    Heaparray[i].run_id = Heaparray[j].run_id;
	    Heaparray[j].key = tmpkey;
	    Heaparray[j].run_id = tmpid;
	}
	
	template<class REC, class KEY, class CMPR>
	inline void merge_heap_kop<REC,KEY,CMPR>::delete_min_and_insert
	(REC *nextelement_same_run)
	{ 
	    if (nextelement_same_run == NULL) {
		Heaparray[1].key = Heaparray[Heapsize].key;
		Heaparray[1].run_id = Heaparray[Heapsize].run_id;
		Heapsize--;
	    } else { 
		UsrObject->copy(&Heaparray[1].key, *nextelement_same_run);
	    }
	    Heapify(1);
	}
	
// Allocate space for the heap
	template<class REC, class KEY, class CMPR>
	inline void merge_heap_kop<REC,KEY,CMPR>::allocate ( memory_size_type size ) {
	    Heaparray = new heap_element<KEY> [size+1];
	    Heapsize  = 0;
	    maxHeapsize = size;
	}
	
// Copy an (initial) element into the heap array
	template<class REC, class KEY, class CMPR>
	inline void merge_heap_kop<REC,KEY,CMPR>::insert (REC *ptr,
							  memory_size_type run_id)
	{
	    UsrObject->copy(&Heaparray[Heapsize+1].key, *ptr);
	    Heaparray[Heapsize+1].run_id = run_id;
	    Heapsize++;
	}
	
// Deallocate the space used by the heap
	template<class REC, class KEY, class CMPR>
	inline void merge_heap_kop<REC,KEY,CMPR>::deallocate () {
	    if (Heaparray){
		delete [] Heaparray; 
		Heaparray=NULL;
	    }
	    Heapsize    = 0;
	    maxHeapsize = 0;
	};
	
//Returns the index of the smallest element out of
//i, the left child of i, and the right child of i
	template<class REC, class KEY, class CMPR>
	inline memory_size_type merge_heap_kop<REC,KEY,CMPR>::get_smallest(
	    memory_size_type i)
	{
	    memory_size_type l,r, smallest;
	    
	    l = Left(i);
	    r = Right(i);
	    
	    smallest = ((l <= Heapsize) && 
			(Heaparray[l].key < Heaparray[i].key)) ? l : i;
	    
	    smallest = ((r <= Heapsize) && 
			(Heaparray[r].key < Heaparray[smallest].key))? r : smallest;
	    
	    return smallest;
	}
	
// This is the primary function; note that we have unfolded the 
// recursion.
	template<class REC, class KEY, class CMPR>
	inline void merge_heap_kop<REC,KEY,CMPR>::Heapify(memory_size_type i) {
	    
	    memory_size_type smallest=get_smallest(i);
	    
	    while (smallest != i) {
		this->Exchange(i,smallest);
		i = smallest;
		smallest = get_smallest(i);
	    }
	}
	
	template<class REC, class KEY, class CMPR>
	void merge_heap_kop<REC,KEY,CMPR>::initialize ( ) {
	    for ( memory_size_type i = Heapsize/2; i >= 1; i--) 
		this->Heapify(i);
	}
	

    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {

// ********************************************************************
// * A key-merge heap that uses a comparison object                   *
// ********************************************************************
	
// The merge_heap_kobj object maintains only the keys in its heap,
// and uses the member function "copy" of the user-provided class CMPR
// to copy these keys from each record. It uses the member function
// "compare" of the user-provided class CMPR to determine the relative
// order of two such keys in the sort order.
	
	template<class REC, class KEY, class CMPR>
	class merge_heap_kobj: public merge_heap_kop<REC,KEY,CMPR>{
	    
	protected: 
	    
	    using merge_heap_kop<REC,KEY,CMPR>::Heapsize;
	    using merge_heap_kop<REC,KEY,CMPR>::Heaparray;
	    using merge_heap_kop<REC,KEY,CMPR>::maxHeapsize;
	    using merge_heap_kop<REC,KEY,CMPR>::UsrObject;
	    
	    inline memory_size_type get_smallest(memory_size_type i);
	    
	    inline void Heapify(memory_size_type);
	public:
	    using merge_heap_kop<REC,KEY,CMPR>::sizeofheap;
	    
	    // Constructor initializes a pointer to the user's comparison object
	    // The object may contain dynamic data although the 'compare' method is const
	    // and therefore inline'able.
	    merge_heap_kobj ( CMPR *cmptr ) :
		merge_heap_kop<REC, KEY, CMPR>(cmptr){};
	    ~merge_heap_kobj(){};
	    
	    // heapify's an initial array of elements
	    void initialize (void);
	    
	    // Delete the current minimum and insert the new item from the same
	    // source / run.
	    inline void delete_min_and_insert(REC *nextelement_same_run);
	    
	};
	
//Returns the index of the smallest element out of
//i, the left child of i, and the right child of i
	template<class REC, class KEY, class CMPR>
	inline memory_size_type merge_heap_kobj<REC,KEY,CMPR>::get_smallest(
	    memory_size_type i)
	{
	    memory_size_type l,r, smallest;
	    
	    l = Left(i);
	    r = Right(i);
	    
	    smallest = ((l <= Heapsize) && 
			(UsrObject->compare(Heaparray[l].key,Heaparray[i].key)<0)) ? l : i;
	    
	    smallest = ((r <= Heapsize) && 
			(UsrObject->compare(Heaparray[r].key,Heaparray[smallest].key)<0)) ?
		r : smallest;
	    
	    return smallest;
	}
	
	template<class REC, class KEY, class CMPR>
	inline void merge_heap_kobj<REC, KEY, CMPR>::Heapify(memory_size_type i) {
	    memory_size_type smallest = get_smallest(i);
	    while (smallest != i) {
		this->Exchange(i,smallest);
		i = smallest;
		smallest = get_smallest(i);
	    }
	}
	
	template<class REC, class KEY, class CMPR>
	inline void merge_heap_kobj<REC,KEY,CMPR>::delete_min_and_insert
	(REC *nextelement_same_run)
	{ 
	    if (nextelement_same_run == NULL) {
		Heaparray[1].key = Heaparray[Heapsize].key;
		Heaparray[1].run_id = Heaparray[Heapsize].run_id;
		Heapsize--;
	    } else { 
		UsrObject->copy(&Heaparray[1].key, *nextelement_same_run);
	    }
	    Heapify(1);
	}
	
	template<class REC, class KEY, class CMPR>
	void merge_heap_kobj<REC, KEY, CMPR>::initialize () {
	    for ( memory_size_type i = Heapsize/2; i >= 1; i--){ Heapify(i); }
	}
	
    }   //  ami namespace

}  //  tpie namespace 

#undef Left
#undef Right
#undef Parent



#endif // _MERGE_HEAP_H
