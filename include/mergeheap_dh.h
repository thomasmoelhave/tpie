//
// File: mergeheap_dh.h
// 
// $Id: mergeheap_dh.h,v 1.8 2004-10-20 08:54:15 jan Exp $	

// This file contains several merge heap templates. 
// Originally written by Rakesh Barve.  

// The heap is basically the heap from CLR except that there is
// provision to exploit the fact that when you are merging you know
// you will be inserting a new element whenever you are doing a
// delete_min.

// Modified by David Hutchinson 2000 03 02

//     - main purpose of the mods is to allow the merge heap to be
//     part of a sort management object. The sort management object
//     contains several procedures and data structures needed for
//     sorting, but the precise versions of these procedures and data
//     structures depend on the sorting approach used (this permits
//     parameterization of the sorting procedure via the sort
//     management object, and avoids having multiple versions of large
//     pieces of code that are highly redundant and difficult to
//     maintain).

//     - move initialization from constructor to an explicit
//     "initialize" member function

//     - add a "comparison object" version of the merge heap
//     object. This allows a comparison object with a "compare" member
//     function to be specified for comparing keys. "Comparison
//     operator" and "comparison function" versions previously
//     existed.

//     - add a set of three (comparison object, operator and function)
//     versions of the merge heap that maintain pointers to the
//     current records at the head of the streams being merged. The
//     previous versions kept the entire corresponding record in the heap.

#ifndef _MERGE_HEAP_DH_H
#define _MERGE_HEAP_DH_H

// Get definitions for working with Unix and Windows
#include <portability.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Macros for left and right.
#define Left(i)   2*i
#define Right(i)  2*i+1
#define Parent(i) i/2

// This is a heap element. Encapsulates the key, along with
// the label run_id indicating the run the key originates from.

template<class KEY>
class heap_element {
public:
    KEY            key;
    TPIE_OS_SIZE_T run_id;
};

// This is a record pointer element. Encapsulates the record pointer,
// along with the label run_id indicating the run the record
// originates from.

template<class REC>
class heap_ptr {
public:
    REC            *recptr;
    TPIE_OS_SIZE_T run_id;
};

// ********************************************************************
// * A record pointer heap that uses a comparison object              *
// ********************************************************************

template<class REC, class CMPR>
class merge_heap_pdh_obj{

    CMPR                *cmp;
    heap_ptr<REC> *Heaparray;
    TPIE_OS_SIZE_T        Heapsize;
    TPIE_OS_SIZE_T        maxHeapsize;

    inline void Exchange(TPIE_OS_SIZE_T i, TPIE_OS_SIZE_T j) {
	REC            *tmpptr;
	unsigned short tmpid;

	tmpptr = Heaparray[i].recptr;
	tmpid  = Heaparray[i].run_id;
    
	Heaparray[i].recptr = Heaparray[j].recptr;
	Heaparray[i].run_id = Heaparray[j].run_id;
    
	Heaparray[j].recptr = tmpptr;
	Heaparray[j].run_id = tmpid;
    };

    inline void Heapify(TPIE_OS_SIZE_T i);

public:

    // Constructor initializes a pointer to the user's comparison object
    // The object may contain dynamic data although the 'compare' method is CONST
    // and therefore inline'able.

    merge_heap_pdh_obj ( CMPR *cmpptr ) {
	cmp = cmpptr;
    }
  
    // Report size of Heap (number of elements)
    TPIE_OS_SIZE_T sizeofheap(void) {return Heapsize;}; 

    // Delete the current minimum and insert the new item from the same
    // source / run.

    inline void delete_min_and_insert(REC *nextelement_same_run){ 
	if (nextelement_same_run == NULL) {
	    Heaparray[1].recptr = Heaparray[Heapsize].recptr;
	    Heaparray[1].run_id = Heaparray[Heapsize].run_id;
	    Heapsize--;
	} else 
	    Heaparray[1].recptr = nextelement_same_run;
	this->Heapify(1);
    };

    // Return the run with the minimum key.
    inline unsigned short get_min_run_id(void) {return Heaparray[1].run_id;};

    // The initialize member function heapify's an initial array of
    // elements
    void initialize ();

    void allocate   (TPIE_OS_SIZE_T size);
    void insert     (REC *ptr, TPIE_OS_SIZE_T run_id);
    void deallocate ();
};

// Allocate space for the heap
template<class REC, class CMPR>
inline void merge_heap_pdh_obj<REC,CMPR>::allocate ( TPIE_OS_SIZE_T size ) {
    Heaparray = new heap_ptr<REC> [size+1];
    Heapsize  = 0;
    maxHeapsize = size;
};

// Copy an (initial) element into the heap array
template<class REC, class CMPR>
inline void merge_heap_pdh_obj<REC,CMPR>::insert ( REC *ptr, TPIE_OS_SIZE_T run_id ) {
    Heaparray[Heapsize+1].recptr = ptr;
    Heaparray[Heapsize+1].run_id = run_id;
    Heapsize++;
    //tp_assert( Heapsize <= maxHeapsize
};

// Deallocate the space used by the heap
template<class REC, class CMPR>
inline void merge_heap_pdh_obj<REC,CMPR>::deallocate () {
    if (Heaparray)
	delete [] Heaparray; 
    Heapsize    = 0;
    maxHeapsize = 0;
};

// This is the primary function; note that we have unfolded the 
// recursion.
template<class REC, class CMPR>
inline void merge_heap_pdh_obj<REC,CMPR>::Heapify(TPIE_OS_SIZE_T i) {

    TPIE_OS_SIZE_T l,r, smallest;

    l = Left(i);
    r = Right(i);

    smallest = ((l <= Heapsize) && (cmp->compare(*Heaparray[l].recptr,*Heaparray[i].recptr)< 0)) ? l : i;

    smallest = ((r <= Heapsize) && 
		(cmp->compare(*Heaparray[r].recptr,*Heaparray[smallest].recptr)<0))? r : smallest;

    while (smallest != i) {
	this->Exchange(i,smallest);
    
	i = smallest;
	l = Left(i);
	r = Right(i);
    
	smallest = ((l <= Heapsize) && 
		    (cmp->compare(*Heaparray[l].recptr,*Heaparray[i].recptr)<0))? l : i;

	smallest =  ((r <= Heapsize) && 
		     (cmp->compare(*Heaparray[r].recptr,*Heaparray[smallest].recptr)<0))? r : smallest;
    }
}

template<class REC, class CMPR>
void merge_heap_pdh_obj<REC,CMPR>::initialize () {
    for ( TPIE_OS_SIZE_T i = Heapsize/2; i >= 1; i--) 
	this->Heapify(i);
}
// ********************************************************************
// * A record pointer heap that uses a comparison operator <          *
// ********************************************************************

template<class REC>
class merge_heap_pdh_op{

    heap_ptr<REC> *Heaparray;
    TPIE_OS_SIZE_T        Heapsize;
    TPIE_OS_SIZE_T        maxHeapsize;

    inline void Exchange(TPIE_OS_SIZE_T i, TPIE_OS_SIZE_T j) {
	REC            *tmpptr;
	unsigned short tmpid;

	tmpptr = Heaparray[i].recptr;
	tmpid  = Heaparray[i].run_id;
    
	Heaparray[i].recptr = Heaparray[j].recptr;
	Heaparray[i].run_id = Heaparray[j].run_id;
    
	Heaparray[j].recptr = tmpptr;
	Heaparray[j].run_id = tmpid;
    };

    inline void Heapify(TPIE_OS_SIZE_T i);

public:

    // Report size of Heap (number of elements)
    TPIE_OS_SIZE_T sizeofheap(void) {return Heapsize;}; 

    // Delete the current minimum and insert the new item from the same
    // source / run.

    inline void delete_min_and_insert(REC *nextelement_same_run){
	if (nextelement_same_run == NULL) {
	    Heaparray[1].recptr = Heaparray[Heapsize].recptr;
	    Heaparray[1].run_id = Heaparray[Heapsize].run_id;
	    Heapsize--;
	} else 
	    Heaparray[1].recptr = nextelement_same_run;
	this->Heapify(1);
    };

    // Return the run with the minimum key.
    inline unsigned short get_min_run_id(void) {return Heaparray[1].run_id;};

    // The initialize member function heapify's an initial array of
    // elements
    void initialize ();

    void allocate   (TPIE_OS_SIZE_T size);
    void insert     (REC *ptr, TPIE_OS_SIZE_T run_id);
    void deallocate ();
};

// Allocate space for the heap
template<class REC>
inline void merge_heap_pdh_op<REC>::allocate ( TPIE_OS_SIZE_T size ) {
    tp_assert( Heaparray = NULL, "Allocating space for heap twice!" );
    Heaparray = new heap_ptr<REC> [size+1];
    Heapsize  = 0;
    maxHeapsize = size;
};

// Copy an (initial) element into the heap array
template<class REC>
inline void merge_heap_pdh_op<REC>::insert ( REC *ptr, TPIE_OS_SIZE_T run_id ) {
    Heaparray[Heapsize+1].recptr = ptr;
    Heaparray[Heapsize+1].run_id = run_id;
    Heapsize++;
    //tp_assert( Heapsize <= maxHeapsize
};

// Deallocate the space used by the heap
template<class REC>
inline void merge_heap_pdh_op<REC>::deallocate () {
    if (Heaparray)
	delete [] Heaparray; 
    Heapsize    = 0;
    maxHeapsize = 0;
};


// This is the primary function; note that we have unfolded the 
// recursion.
template<class REC>
inline void merge_heap_pdh_op<REC>::Heapify(TPIE_OS_SIZE_T i) {

    TPIE_OS_SIZE_T l,r, smallest;

    l = Left(i);
    r = Right(i);

    smallest = ((l <= Heapsize) && 
		(*Heaparray[l].recptr < *Heaparray[i].recptr)) ? l : i;

    smallest = ((r <= Heapsize) && 
		(*Heaparray[r].recptr < *Heaparray[smallest].recptr))? r : smallest;

    while (smallest != i) {
	this->Exchange(i,smallest);

	i = smallest;
	l = Left(i);
	r = Right(i);
    
	smallest = ((l <= Heapsize) && 
		    (*Heaparray[l].recptr < *Heaparray[i].recptr))? l : i;
    
	smallest =  ((r <= Heapsize) && 
		     (*Heaparray[r].recptr < *Heaparray[smallest].recptr))? r : smallest;
    }
}

template<class REC>
void merge_heap_pdh_op<REC>::initialize () {
    for ( TPIE_OS_SIZE_T i = Heapsize/2; i >= 1; i--) 
	this->Heapify(i);
}

// ********************************************************************
// * A record pointer heap that uses a comparison function            *
// ********************************************************************

template<class REC>
class merge_heap_pdh_cmp {

  class heap_ptr<REC>     *Heaparray;
  TPIE_OS_SIZE_T            Heapsize;
  TPIE_OS_SIZE_T            maxHeapsize;

  // The constructor will provide the comparison function
  int (*cmp)(CONST REC&, CONST REC&);

  inline void Exchange(TPIE_OS_SIZE_T i, TPIE_OS_SIZE_T j){
    REC            *tmpptr;
    unsigned short tmpid;

    tmpptr = Heaparray[i].recptr;
    tmpid  = Heaparray[i].run_id;
  
    Heaparray[i].recptr = Heaparray[j].recptr;
    Heaparray[i].run_id = Heaparray[j].run_id;

    Heaparray[j].recptr = tmpptr;
    Heaparray[j].run_id = tmpid;
  };

  inline void Heapify(TPIE_OS_SIZE_T i);

public:

  // Constructor
  merge_heap_pdh_cmp( int (*cmp_f)(CONST REC &, CONST REC &) ) {    
    cmp = cmp_f;
  };

  // Report size of Heap (number of elements)
  TPIE_OS_SIZE_T sizeofheap(void) {return Heapsize;}; 
  
  // Delete the current minimum and insert the new item from
  // the same source / run.

  inline void delete_min_and_insert(REC *nextelement_same_run) {  
    if (nextelement_same_run == NULL) {
      Heaparray[1].recptr = Heaparray[Heapsize].recptr;
      Heaparray[1].run_id = Heaparray[Heapsize].run_id;
      Heapsize--;
    } else Heaparray[1].recptr = nextelement_same_run;
    this->Heapify(1);
  };

  //Return the run with the minimum key.
  inline unsigned short get_min_run_id(void) {return Heaparray[1].run_id;};

  // create initial heap by heapifying the array of keys provided

  void initialize();

  void allocate   (TPIE_OS_SIZE_T size);
  void insert     (REC *ptr, TPIE_OS_SIZE_T run_id);
  void deallocate ();
};

// Allocate space for the heap
template<class REC>
inline void merge_heap_pdh_cmp<REC>::allocate ( TPIE_OS_SIZE_T size ) {
    Heaparray = new heap_ptr<REC> [size+1];
    Heapsize  = 0;
    maxHeapsize = size;
};

// Copy an (initial) element into the heap array
template<class REC>
inline void merge_heap_pdh_cmp<REC>::insert ( REC *ptr, TPIE_OS_SIZE_T run_id ) {
    Heaparray[Heapsize+1].recptr = ptr;
    Heaparray[Heapsize+1].run_id = run_id;
    Heapsize++;
    //tp_assert( Heapsize <= maxHeapsize
};

// Deallocate the space used by the heap
template<class REC>
inline void merge_heap_pdh_cmp<REC>::deallocate () {
    if (Heaparray)
       delete [] Heaparray; 
    Heapsize    = 0;
    maxHeapsize = 0;
};


// This is the primary function; note that we have unfolded the 
// recursion.
template<class REC>
inline void merge_heap_pdh_cmp<REC>::Heapify(TPIE_OS_SIZE_T i) {
  TPIE_OS_SIZE_T l,r, smallest;
  
  l = Left(i);
  r = Right(i);

  smallest = ((l <= Heapsize) && (cmp(*Heaparray[l].recptr,*Heaparray[i].recptr)< 0)) ? l : i;

  smallest = ((r <= Heapsize) && 
	      (cmp(*Heaparray[r].recptr,*Heaparray[smallest].recptr)<0))? r : smallest;

  while (smallest != i) {
    this->Exchange(i,smallest);
    
    i = smallest;
    l = Left(i);
    r = Right(i);
    
    smallest = ((l <= Heapsize) && 
		(cmp(*Heaparray[l].recptr,*Heaparray[i].recptr)<0))? l : i;

    smallest =  ((r <= Heapsize) && 
		 (cmp(*Heaparray[r].recptr,*Heaparray[smallest].recptr)<0))? r : smallest;

  }
}

// create initial heap by heapifying the array of keys provided

template<class REC>
void merge_heap_pdh_cmp<REC>::initialize () {
  for ( TPIE_OS_SIZE_T i = Heapsize/2; i >= 1; i--) 
    this->Heapify(i);
}

// ********************************************************************
// * A merge heap that uses a comparison object                       *
// ********************************************************************

template<class REC, class CMPR>
class merge_heap_dh_obj{

    CMPR                    *cmp;
    heap_element<REC> *Heaparray;
    TPIE_OS_SIZE_T            Heapsize;
    TPIE_OS_SIZE_T            maxHeapsize;
    inline void Exchange(TPIE_OS_SIZE_T i, TPIE_OS_SIZE_T j) {

	REC tmpkey;
	TPIE_OS_SIZE_T tmpid;

	tmpkey = Heaparray[i].key;
	tmpid = Heaparray[i].run_id;
    
	Heaparray[i].key = Heaparray[j].key;
	Heaparray[i].run_id = Heaparray[j].run_id;
    
	Heaparray[j].key = tmpkey;
	Heaparray[j].run_id = tmpid;
    };

    inline void Heapify(TPIE_OS_SIZE_T i);

public:
    // Constructor initializes a pointer to the user's comparison object
    // The object may contain dynamic data although the 'compare' method is CONST
    // and therefore inline'able.

    merge_heap_dh_obj ( CMPR *cmpptr ) {
	cmp = cmpptr;
    }

    // Report size of Heap (number of elements)
    TPIE_OS_SIZE_T sizeofheap(void) {return Heapsize;}; 

    // Delete the current minimum and insert the new item from the same
    // source / run.

    inline void delete_min_and_insert(REC *nextelement_same_run){
  
	if (nextelement_same_run == NULL) {
	    Heaparray[1].key = Heaparray[Heapsize].key;
	    Heaparray[1].run_id = Heaparray[Heapsize].run_id;
	    Heapsize--;
	} else 
	    Heaparray[1].key = *nextelement_same_run;
	this->Heapify(1);
    };

    // Return the run with the minimum key.
    inline TPIE_OS_SIZE_T get_min_run_id(void) {return Heaparray[1].run_id;};

    // The initialize member function heapify's an initial array of
    // elements
    void initialize ();

    void allocate   (TPIE_OS_SIZE_T size);
    void insert     (REC *ptr, TPIE_OS_SIZE_T run_id);
    void deallocate ();
};

// Allocate space for the heap
template<class REC, class CMPR>
inline void merge_heap_dh_obj<REC,CMPR>::allocate ( TPIE_OS_SIZE_T size ) {
    Heaparray = new heap_element<REC> [size+1];
    Heapsize  = 0;
    maxHeapsize = size;
};

// Copy an (initial) element into the heap array
template<class REC, class CMPR>
inline void merge_heap_dh_obj<REC,CMPR>::insert ( REC *ptr, TPIE_OS_SIZE_T run_id ) {
    Heaparray[Heapsize+1].key    = *ptr;
    Heaparray[Heapsize+1].run_id = run_id;
    Heapsize++;
    //tp_assert( Heapsize <= maxHeapsize
};

// Deallocate the space used by the heap
template<class REC, class CMPR>
inline void merge_heap_dh_obj<REC,CMPR>::deallocate () {
    if (Heaparray)
	delete [] Heaparray; 
    Heapsize    = 0;
    maxHeapsize = 0;
};

// This is the primary function; note that we have unfolded the 
// recursion.
template<class REC, class CMPR>
inline void merge_heap_dh_obj<REC,CMPR>::Heapify(TPIE_OS_SIZE_T i) {

    TPIE_OS_SIZE_T l,r, smallest;

    l = Left(i);
    r = Right(i);

    smallest = ((l <= Heapsize) && (cmp->compare(Heaparray[l].key,Heaparray[i].key)< 0)) ? l : i;

    smallest = ((r <= Heapsize) && 
		(cmp->compare(Heaparray[r].key,Heaparray[smallest].key)<0))? r : smallest;

    while (smallest != i) {
	this->Exchange(i,smallest);
    
	i = smallest;
	l = Left(i);
	r = Right(i);
    
	smallest = ((l <= Heapsize) && 
		    (cmp->compare(Heaparray[l].key,Heaparray[i].key)<0))? l : i;

	smallest =  ((r <= Heapsize) && 
		     (cmp->compare(Heaparray[r].key,Heaparray[smallest].key)<0))? r : smallest;
    }
}

template<class REC, class CMPR>
void merge_heap_dh_obj<REC,CMPR>::initialize () {
    for ( TPIE_OS_SIZE_T i = Heapsize/2; i >= 1; i--) 
	this->Heapify(i);
}

// ********************************************************************
// * A merge heap that uses a comparison operator <                   *
// ********************************************************************

template<class REC>
class merge_heap_dh_op{

    heap_element<REC> *Heaparray;
    TPIE_OS_SIZE_T            Heapsize;
    TPIE_OS_SIZE_T            maxHeapsize;
    inline void Exchange(TPIE_OS_SIZE_T i, TPIE_OS_SIZE_T j) {

	REC tmpkey;
	TPIE_OS_SIZE_T tmpid;

	tmpkey = Heaparray[i].key;
	tmpid = Heaparray[i].run_id;
    
	Heaparray[i].key = Heaparray[j].key;
	Heaparray[i].run_id = Heaparray[j].run_id;
    
	Heaparray[j].key = tmpkey;
	Heaparray[j].run_id = tmpid;
    };

    inline void Heapify(TPIE_OS_SIZE_T i);

public:

    // Report size of Heap (number of elements)
    TPIE_OS_SIZE_T sizeofheap(void) {return Heapsize;}; 

    // Delete the current minimum and insert the new item from the same
    // source / run.

    inline void delete_min_and_insert(REC *nextelement_same_run){
  
	if (nextelement_same_run == NULL) {
	    Heaparray[1].key = Heaparray[Heapsize].key;
	    Heaparray[1].run_id = Heaparray[Heapsize].run_id;
	    Heapsize--;
	} else 
	    Heaparray[1].key = *nextelement_same_run;
	this->Heapify(1);
    };

    // Return the run with the minimum key.
    inline TPIE_OS_SIZE_T get_min_run_id(void) {return Heaparray[1].run_id;};

    // The initialize member function heapify's an initial array of
    // elements
    void initialize ();

    void allocate   (TPIE_OS_SIZE_T size);
    void insert     (REC *ptr, TPIE_OS_SIZE_T run_id);
    void deallocate ();
};

// Allocate space for the heap
template<class REC>
inline void merge_heap_dh_op<REC>::allocate ( TPIE_OS_SIZE_T size ) {
    Heaparray = new heap_element<REC> [size+1];
    Heapsize  = 0;
    maxHeapsize = size;
};

// Copy an (initial) element into the heap array
template<class REC>
inline void merge_heap_dh_op<REC>::insert ( REC *ptr, TPIE_OS_SIZE_T run_id ) {
    Heaparray[Heapsize+1].key    = *ptr;
    Heaparray[Heapsize+1].run_id = run_id;
    Heapsize++;
    //tp_assert( Heapsize <= maxHeapsize
};

// Deallocate the space used by the heap
template<class REC>
inline void merge_heap_dh_op<REC>::deallocate () {
    if (Heaparray)
	delete [] Heaparray; 
    Heapsize    = 0;
    maxHeapsize = 0;
};

// This is the primary function; note that we have unfolded the 
// recursion.
template<class REC>
inline void merge_heap_dh_op<REC>::Heapify(TPIE_OS_SIZE_T i) {

    TPIE_OS_SIZE_T l,r, smallest;

    l = Left(i);
    r = Right(i);

    smallest = ((l <= Heapsize) && 
		(Heaparray[l].key < Heaparray[i].key)) ? l : i;

    smallest = ((r <= Heapsize) && 
		(Heaparray[r].key < Heaparray[smallest].key))? r : smallest;

    while (smallest != i) {
	this->Exchange(i,smallest);

	i = smallest;
	l = Left(i);
	r = Right(i);
    
	smallest = ((l <= Heapsize) && 
		(Heaparray[l].key < Heaparray[i].key))? l : i;
    
	smallest =  ((r <= Heapsize) && 
		(Heaparray[r].key < Heaparray[smallest].key))? r : smallest;
    }
}

template<class REC>
void merge_heap_dh_op<REC>::initialize () {
    for ( TPIE_OS_SIZE_T i = Heapsize/2; i >= 1; i--) 
	this->Heapify(i);
}


// ********************************************************************
// * A merge heap that uses a comparison function                     *
// ********************************************************************

template<class REC>
class merge_heap_dh_cmp {

  class heap_element<REC> *Heaparray;
  TPIE_OS_SIZE_T            Heapsize;
  TPIE_OS_SIZE_T            maxHeapsize;

  // The constructor will provide this comparison function
  int (*cmp)(CONST REC&, CONST REC&);

  inline void Exchange(TPIE_OS_SIZE_T i, TPIE_OS_SIZE_T j){

    REC tmpkey;
    unsigned short tmpid;

    tmpkey = Heaparray[i].key;
    tmpid = Heaparray[i].run_id;
  
    Heaparray[i].key = Heaparray[j].key;
    Heaparray[i].run_id = Heaparray[j].run_id;

    Heaparray[j].key = tmpkey;
    Heaparray[j].run_id = tmpid;
  };

  inline void Heapify(TPIE_OS_SIZE_T i);

public:

  // Constructor
  merge_heap_dh_cmp( int (*cmp_f)(CONST REC&, CONST REC&) ) {    
    cmp = cmp_f;
  };

  // Report size of Heap (number of elements)
  TPIE_OS_SIZE_T sizeofheap(void) {return Heapsize;}; 
  
  // Delete the current minimum and insert the new item from
  // the same source / run.

  inline void delete_min_and_insert(REC *nextelement_same_run) {
  
    if (nextelement_same_run == NULL) {
      Heaparray[1].key = Heaparray[Heapsize].key;
      Heaparray[1].run_id = Heaparray[Heapsize].run_id;
      Heapsize--;
    } else Heaparray[1].key = *nextelement_same_run;
    this->Heapify(1);
  };

  //Return the run with the minimum key.
  inline unsigned short get_min_run_id(void) {return Heaparray[1].run_id;};

  // create initial heap by heapifying the array of keys provided

  void initialize( );

  void allocate   (TPIE_OS_SIZE_T size);
  void insert     (REC *ptr, TPIE_OS_SIZE_T run_id);
  void deallocate ();
};

// Allocate space for the heap
template<class REC>
inline void merge_heap_dh_cmp<REC>::allocate ( TPIE_OS_SIZE_T size ) {
    Heaparray = new heap_element<REC> [size+1];
    Heapsize  = 0;
    maxHeapsize = size;
};

// Copy an (initial) element into the heap array
template<class REC>
inline void merge_heap_dh_cmp<REC>::insert ( REC *ptr, TPIE_OS_SIZE_T run_id ) {
    Heaparray[Heapsize+1].key    = *ptr;
    Heaparray[Heapsize+1].run_id = run_id;
    Heapsize++;
    //tp_assert( Heapsize <= maxHeapsize
};

// Deallocate the space used by the heap
template<class REC>
inline void merge_heap_dh_cmp<REC>::deallocate () {
    if (Heaparray)
       delete [] Heaparray; 
    Heapsize    = 0;
    maxHeapsize = 0;
};


// This is the primary function; note that we have unfolded the 
// recursion.
template<class REC>
inline void merge_heap_dh_cmp<REC>::Heapify(TPIE_OS_SIZE_T i) {
  TPIE_OS_SIZE_T l,r, smallest;
  
  l = Left(i);
  r = Right(i);

  smallest = ((l <= Heapsize) && (cmp(Heaparray[l].key,Heaparray[i].key)< 0)) ? l : i;

  smallest = ((r <= Heapsize) && 
	      (cmp(Heaparray[r].key,Heaparray[smallest].key)<0))? r : smallest;

  while (smallest != i) {
    this->Exchange(i,smallest);
    
    i = smallest;
    l = Left(i);
    r = Right(i);
    
    smallest = ((l <= Heapsize) && 
		(cmp(Heaparray[l].key,Heaparray[i].key)<0))? l : i;

    smallest =  ((r <= Heapsize) && 
		 (cmp(Heaparray[r].key,Heaparray[smallest].key)<0))? r : smallest;

  }
}

// create initial heap by heapifying the array of keys provided

template<class REC>
void merge_heap_dh_cmp<REC>::initialize () {
  for ( TPIE_OS_SIZE_T i = Heapsize/2; i >= 1; i--) 
    this->Heapify(i);
}


// ********************************************************************
// * A key-merge heap that uses a comparison operator <                   *
// ********************************************************************

// The merge_heap_dh_kop object maintains only the keys in its heap,
// and uses the member function "copy" of the user-provided class CMPR
// to copy these keys from each record.
 
template<class REC, class KEY, class CMPR>
class merge_heap_dh_kop{

    CMPR                               *UsrObject;
    heap_element<KEY> *Heaparray;
    TPIE_OS_SIZE_T                       Heapsize;
    TPIE_OS_SIZE_T                       maxHeapsize;

    inline void Exchange(TPIE_OS_SIZE_T i, TPIE_OS_SIZE_T j) {
	KEY            tmpkey;
	unsigned short tmpid;

	tmpkey = Heaparray[i].key;
	tmpid  = Heaparray[i].run_id;
    
	Heaparray[i].key    = Heaparray[j].key;
	Heaparray[i].run_id = Heaparray[j].run_id;
    
	Heaparray[j].key    = tmpkey;
	Heaparray[j].run_id = tmpid;
    };

    inline void Heapify(TPIE_OS_SIZE_T i);

public:

    // Constructor initializes a pointer to the user's comparison object
    // The object may contain dynamic data although the 'copy' method is CONST
    // and therefore inline'able.

    merge_heap_dh_kop ( CMPR *cmpptr ) {
	UsrObject = cmpptr;
    }

    // Report size of Heap (number of elements)
    TPIE_OS_SIZE_T sizeofheap(void) {return Heapsize;}; 

    // Delete the current minimum and insert the new item from the same
    // source / run.

    inline void delete_min_and_insert( REC *nextelement_same_run ){
	if (nextelement_same_run == NULL) {
	    Heaparray[1].key = Heaparray[Heapsize].key;
	    Heaparray[1].run_id = Heaparray[Heapsize].run_id;
	    Heapsize--;
	} else 
	    UsrObject->copy(&Heaparray[1].key, *nextelement_same_run);
	this->Heapify(1);
    };

    // Return the run with the minimum key.
    inline unsigned short get_min_run_id(void) {return Heaparray[1].run_id;};

    // The initialize member function heapify's an initial array of
    // elements
    void initialize ();
    void allocate   (TPIE_OS_SIZE_T size);
    void insert     (REC *ptr, TPIE_OS_SIZE_T run_id);
    void deallocate ();
};

// Allocate space for the heap
template<class REC, class KEY, class CMPR>
inline void merge_heap_dh_kop<REC,KEY,CMPR>::allocate ( TPIE_OS_SIZE_T size ) {
    Heaparray = new heap_element<KEY> [size+1];
    Heapsize  = 0;
    maxHeapsize = size;
};

// Copy an (initial) element into the heap array
template<class REC, class KEY, class CMPR>
inline void merge_heap_dh_kop<REC,KEY,CMPR>::insert ( REC *ptr, TPIE_OS_SIZE_T run_id ) {
    UsrObject->copy(&Heaparray[Heapsize+1].key, *ptr);
    Heaparray[Heapsize+1].run_id = run_id;
    Heapsize++;
    //tp_assert( Heapsize <= maxHeapsize
};

// Deallocate the space used by the heap
template<class REC, class KEY, class CMPR>
inline void merge_heap_dh_kop<REC,KEY,CMPR>::deallocate () {
    if (Heaparray)
	delete [] Heaparray; 
    Heapsize    = 0;
    maxHeapsize = 0;
};

// This is the primary function; note that we have unfolded the 
// recursion.
template<class REC, class KEY, class CMPR>
inline void merge_heap_dh_kop<REC,KEY,CMPR>::Heapify(TPIE_OS_SIZE_T i) {

    TPIE_OS_SIZE_T l,r, smallest;

    l = Left(i);
    r = Right(i);

    smallest = ((l <= Heapsize) && 
		(Heaparray[l].key < Heaparray[i].key)) ? l : i;

    smallest = ((r <= Heapsize) && 
		(Heaparray[r].key < Heaparray[smallest].key))? r : smallest;

    while (smallest != i) {
	this->Exchange(i,smallest);

	i = smallest;
	l = Left(i);
	r = Right(i);
    
	smallest = ((l <= Heapsize) && 
		    (Heaparray[l].key < Heaparray[i].key))? l : i;
    
	smallest =  ((r <= Heapsize) && 
		     (Heaparray[r].key < Heaparray[smallest].key))? r : smallest;
    }
}

template<class REC, class KEY, class CMPR>
void merge_heap_dh_kop<REC,KEY,CMPR>::initialize ( ) {
    for ( TPIE_OS_SIZE_T i = Heapsize/2; i >= 1; i--) 
	this->Heapify(i);
}

// ********************************************************************
// * A key-merge heap that uses a comparison object                   *
// ********************************************************************

// The merge_heap_dh_kobj object maintains only the keys in its heap,
// and uses the member function "copy" of the user-provided class CMPR
// to copy these keys from each record. It uses the member function
// "compare" of the user-provided class CMPR to determine the relative
// order of two such keys in the sort order.
 
template<class REC, class KEY, class CMPR>
class merge_heap_dh_kobj{

    CMPR                                *UsrObject;
    heap_element<KEY> *Heaparray;
    TPIE_OS_SIZE_T                        Heapsize;
    TPIE_OS_SIZE_T                        maxHeapsize;

    inline void Exchange(TPIE_OS_SIZE_T i, TPIE_OS_SIZE_T j) {
	KEY            tmpkey;
	unsigned short tmpid;

	tmpkey = Heaparray[i].key;
	tmpid  = Heaparray[i].run_id;
    
	Heaparray[i].key    = Heaparray[j].key;
	Heaparray[i].run_id = Heaparray[j].run_id;
    
	Heaparray[j].key    = tmpkey;
	Heaparray[j].run_id = tmpid;
    };

    inline void Heapify(TPIE_OS_SIZE_T i);

public:

    // Constructor initializes a pointer to the user's comparison object
    // The object may contain dynamic data although the 'copy' and
    // 'compare' methods are CONST and therefore inline'able.

    merge_heap_dh_kobj ( CMPR *cmpptr ) {
	UsrObject = cmpptr;
    }

    // Report size of Heap (number of elements)
    TPIE_OS_SIZE_T sizeofheap(void) {return Heapsize;}; 

    // Delete the current minimum and insert the new item from the same
    // source / run.

    inline void delete_min_and_insert( REC *nextelement_same_run ){
	if (nextelement_same_run == NULL) {
	    Heaparray[1].key = Heaparray[Heapsize].key;
	    Heaparray[1].run_id = Heaparray[Heapsize].run_id;
	    Heapsize--;
	} else 
	    UsrObject->copy(&Heaparray[1].key, *nextelement_same_run);
	this->Heapify(1);
    };

    // Return the run with the minimum key.
    inline unsigned short get_min_run_id(void) {return Heaparray[1].run_id;};

    // initialize heapify's the initial array of elements
    void initialize ();

    // allocate allocates space for the heap
    void allocate   (TPIE_OS_SIZE_T size);

    // insert copies an element into the heap array
    void insert     (REC *ptr, TPIE_OS_SIZE_T run_id);

    // deallocate deallocates the space used by the heap
    void deallocate ();
};

// Allocate space for the heap
template<class REC, class KEY, class CMPR>
inline void merge_heap_dh_kobj<REC,KEY,CMPR>::allocate( TPIE_OS_SIZE_T size ) {
    Heaparray = new heap_element<KEY> [size+1];
    Heapsize  = 0;
    maxHeapsize = size;
};

// Copy an (initial) element into the heap array
template<class REC, class KEY, class CMPR>
inline void merge_heap_dh_kobj<REC,KEY,CMPR>::insert( REC *ptr, TPIE_OS_SIZE_T run_id ) {
    UsrObject->copy(&Heaparray[Heapsize+1].key, *ptr);
    Heaparray[Heapsize+1].run_id = run_id;
    Heapsize++;
    //tp_assert( Heapsize <= maxHeapsize
};

// Deallocate the space used by the heap
template<class REC, class KEY, class CMPR>
inline void merge_heap_dh_kobj<REC,KEY,CMPR>::deallocate() {
    if (Heaparray)
	delete [] Heaparray; 
    Heapsize    = 0;
    maxHeapsize = 0;
};

// This is the primary function; note that we have unfolded the 
// recursion.
template<class REC, class KEY, class CMPR>
inline void merge_heap_dh_kobj<REC,KEY,CMPR>::Heapify(TPIE_OS_SIZE_T i) {

    TPIE_OS_SIZE_T l,r, smallest;


    l = Left(i);
    r = Right(i);

    smallest = ((l <= Heapsize) && 
		(UsrObject->compare(Heaparray[l].key,Heaparray[i].key)<0)) ? l : i;
    smallest = ((r <= Heapsize) && 
		(UsrObject->compare(Heaparray[r].key,Heaparray[smallest].key)<0))? r : smallest;
    while (smallest != i) {
	this->Exchange(i,smallest);

	i = smallest; 
	l = Left(i);
	r = Right(i); 
    
	smallest = ((l <= Heapsize) && 
		    (UsrObject->compare(Heaparray[l].key,Heaparray[i].key)<0))? l : i;
    
	smallest =  ((r <= Heapsize) && 
		     (UsrObject->compare(Heaparray[r].key,Heaparray[smallest].key)<0))? r : smallest;
    };
}

template<class REC, class KEY, class CMPR>
inline void merge_heap_dh_kobj<REC,KEY,CMPR>::initialize ( ) {
    for ( TPIE_OS_SIZE_T i = Heapsize/2; i >= 1; i--) 
	this->Heapify(i);
}

#endif // _MERGE_HEAP_DH_H
