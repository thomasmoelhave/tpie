// Copyright (c) 2005 Andrew Danner
//
// File: internal_sorter.h
// Author: Andrew Danner <adanner@cs.duke.edu>
// Created: 28 Jun 2005
//
// Internal sorter class that can be used within AMI_sort() on small
// streams/substreams
//
// $Id: internal_sort.h,v 1.6 2006-05-20 22:19:53 adanner Exp $
//
#ifndef _INTERNAL_SORT_H
#define _INTERNAL_SORT_H
 
// Get definitions for working with Unix and Windows
#include <portability.h>
#include <quicksort.h>

// Use our quicksort, or the sort from STL
#ifdef TPIE_USE_STL_SORT
// portability.h includes <algorithm> for us in the case of STL sort
#include <comparator.h> //to convert TPIE comparisons to STL
#endif

namespace tpie {

    namespace ami {

// The base class. This class does not have a sort() function, so it
// cannot be used directly
	template<class T>
	class Internal_Sorter_Base {
	    
	protected:
	    T* ItemArray;        //Array that holds items to be sorted
	    TPIE_OS_SIZE_T len;  //length of ItemArray
	    
	public:
	    //  Constructor
	    Internal_Sorter_Base(void): ItemArray(NULL), len(0) {
		//  No code in this constructor.
	    };
	    
	    virtual ~Internal_Sorter_Base(void); // Destructor
    
	    //Allocate array that can hold nItems
	    void allocate(TPIE_OS_SIZE_T nItems);
	    
	    void deallocate(void); //Clean up internal array
	    
	    // Maximum number of Items that can be sorted using memSize bytes
	    TPIE_OS_SIZE_T MaxItemCount(TPIE_OS_SIZE_T memSize);
	    
	    // Memory usage per sort item
	    TPIE_OS_SIZE_T space_per_item();
	    
	    // Fixed memory usage overhead per class instantiation
	    TPIE_OS_SIZE_T space_overhead();
	    
	private:
	    // Prohibit these
	    Internal_Sorter_Base(const Internal_Sorter_Base<T>& other);
	    Internal_Sorter_Base<T> operator=(const Internal_Sorter_Base<T>& other);
	};
	
	template<class T>
	Internal_Sorter_Base<T>::~Internal_Sorter_Base(void) {
	    //In case someone forgot to call deallocate()
	    if(ItemArray){
		delete [] ItemArray;
		ItemArray=NULL;
	    }
	}
	
	template<class T>
	inline void Internal_Sorter_Base<T>::allocate(TPIE_OS_SIZE_T nitems) {
	    len=nitems;
	    ItemArray = new T[len];
	}
	
	template<class T>
	inline void Internal_Sorter_Base<T>::deallocate(void) {
	    if(ItemArray){
		delete [] ItemArray;
		ItemArray=NULL;
		len=0;
	    }
	}
	
	template<class T>
	inline TPIE_OS_SIZE_T Internal_Sorter_Base<T>::MaxItemCount(TPIE_OS_SIZE_T memSize) {
	    //Space available for items
	    TPIE_OS_SIZE_T memAvail=memSize-space_overhead();
	    
	    if (memAvail < space_per_item() ){
		return 0; 
	    }
	    else{ 
		return memAvail/space_per_item(); 
	    }
	}
	
	
	template<class T>
	inline TPIE_OS_SIZE_T Internal_Sorter_Base<T>::space_overhead(void) {
	    // Space usage independent of space_per_item
	    // accounts MM_manager space overhead on "new" call
	    return MM_manager.space_overhead();
	}

	template<class T>
	inline TPIE_OS_SIZE_T Internal_Sorter_Base<T>::space_per_item(void) {
	    return sizeof(T);
	}
	
    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {

// *********************************************************************
// *                                                                   *
// * Operator based Internal Sorter.                                   *
// *                                                                   *
// *********************************************************************
	
	template<class T>
	class Internal_Sorter_Op: public Internal_Sorter_Base<T>{

	protected:
	    using Internal_Sorter_Base<T>::len;
	    using Internal_Sorter_Base<T>::ItemArray;
	    
	public:
	    //  Constructor/Destructor
	    Internal_Sorter_Op() {
		//  No code in this constructor.
	    };

	    ~Internal_Sorter_Op() {
		//  No code in this destructor.
	    };
	    
	    using Internal_Sorter_Base<T>::space_overhead;
	    
	    //Sort nItems from input stream and write to output stream
	    err sort(stream<T>* InStr, stream<T>* OutStr, TPIE_OS_SIZE_T nItems);
	    
	private:
	    // Prohibit these
	    Internal_Sorter_Op(const Internal_Sorter_Op<T>& other);
	    Internal_Sorter_Op<T> operator=(const Internal_Sorter_Op<T>& other);
	};
	
// Read nItems sequentially from InStr, starting at the current file
// position. Write the sorted output to OutStr, starting from the current
// file position.
	template<class T>
	err Internal_Sorter_Op<T>::sort(stream<T>* InStr, stream<T>* OutStr, TPIE_OS_SIZE_T nItems){
	    
	    err ae  = NO_ERROR;
	    T    *next_item;
	    TPIE_OS_SIZE_T i = 0;
	    
	    // make sure we called allocate earlier
	    if (ItemArray==NULL){
		return NULL_POINTER;
	    }
	    
	    tp_assert ( nItems <= len, "nItems more than interal buffer size.");
	    
	    // Read a memory load out of the input stream one item at a time,
	    for (i = 0; i < nItems; i++) {
		if ((ae=InStr->read_item (&next_item)) != NO_ERROR) {
		    
		    TP_LOG_FATAL_ID ("Internal sort: AMI read error " << ae);
		    
		    return ae;
		}
		
		ItemArray[i] = *next_item;
	    }

	    //Sort the array.
#ifdef TPIE_USE_STL_SORT
	    TP_LOG_DEBUG_ID("calling STL sort for " << nItems << " items");
	    std::sort(ItemArray, ItemArray+nItems);
#else 
	    TP_LOG_DEBUG("calling quick_sort_op for " << nItems << " items\n");
	    quick_sort_op<T> (ItemArray, nItems);
#endif
	    
	    if(InStr==OutStr){ //Do the right thing if we are doing 2x sort
		//Internal sort objects should probably be re-written so that
		//the interface is cleaner and they don't have to worry about I/O
		InStr->truncate(0); //delete original items
		InStr->seek(0); //rewind
	    }

	    //  Write sorted array to OutStr
	    for (i = 0; i < nItems; i++) {
		if ((ae = OutStr->write_item(ItemArray[i])) != NO_ERROR) {
		    
		    TP_LOG_FATAL_ID ("Internal Sorter: AMI write error " << ae );
		    
		    return ae;
		}
	    }
	    
	    return NO_ERROR;
	}

    }  //  ami namespace

}  //  tpie namespace


namespace tpie {

    namespace ami {
	
// *********************************************************************
// *                                                                   *
// * Comparison object based Internal Sorter.                          *
// *                                                                   *
// *********************************************************************
	
	template<class T, class CMPR>
	class Internal_Sorter_Obj: public Internal_Sorter_Base<T>{

	protected:
	    using Internal_Sorter_Base<T>::ItemArray;
	    using Internal_Sorter_Base<T>::len;
	    CMPR *cmp_o; //Comparison object used for sorting
	    
	public:
	    //Constructor/Destructor
	    Internal_Sorter_Obj(CMPR* cmp) :cmp_o(cmp) {};
	    ~Internal_Sorter_Obj(){};
	    
	    using Internal_Sorter_Base<T>::space_overhead;
	    
	    //Sort nItems from input stream and write to output stream
	    err sort(stream<T>* InStr, stream<T>* OutStr, TPIE_OS_SIZE_T nItems);
	    
	private:
	    // Prohibit these
	    Internal_Sorter_Obj(const Internal_Sorter_Obj<T,CMPR>& other);
	    Internal_Sorter_Obj<T,CMPR> operator=(const Internal_Sorter_Obj<T,CMPR>& other);
	};
	
// Read nItems sequentially from InStr, starting at the current file
// position. Write the sorted output to OutStr, starting from the current
// file position.
	template<class T, class CMPR>
	err Internal_Sorter_Obj<T, CMPR>::sort(stream<T>* InStr, stream<T>* OutStr, TPIE_OS_SIZE_T nItems) {
	    
	    err ae = NO_ERROR;
	    T    *next_item;
	    TPIE_OS_SIZE_T i = 0;

	    //make sure we called allocate earlier
	    if (ItemArray==NULL) { 
		return NULL_POINTER;
	    }
	    
	    tp_assert ( nItems <= len, "nItems more than interal buffer size.");
	    
	    // Read a memory load out of the input stream one item at a time,
	    for (i = 0; i < nItems; i++) {
		if ((ae=InStr->read_item (&next_item)) != NO_ERROR) {
		    
		    TP_LOG_FATAL_ID ("Internal sort: AMI read error " << ae);

		    return ae;
		}
		
		ItemArray[i] = *next_item;
	    }

	    //Sort the array.
#ifdef TPIE_USE_STL_SORT
	    TP_LOG_DEBUG_ID("calling STL sort for " << nItems << " items");
	    TP_LOG_DEBUG("converting TPIE comparison object to STL\n");
	    std::sort(ItemArray, ItemArray+nItems, TPIE2STL_cmp<T,CMPR>(cmp_o));
#else 
	    TP_LOG_DEBUG("calling quick_sort_obj for " << nItems << " items\n");
	    quick_sort_obj<T> (ItemArray, nItems, cmp_o);
#endif
	    
	    if (InStr==OutStr) { //Do the right thing if we are doing 2x sort
		//Internal sort objects should probably be re-written so that
		//the interface is cleaner and they don't have to worry about I/O
		InStr->truncate(0); //delete original items
		InStr->seek(0); //rewind
	    }

	    //Write sorted array to OutStr
	    for (i = 0; i < nItems; i++) {
		if ((ae = OutStr->write_item(ItemArray[i])) != NO_ERROR) {
		    
		    TP_LOG_FATAL_ID ("Internal Sorter: AMI write error" << ae );
		    
		    return ae;
		}		
	    }
	    
	    return NO_ERROR;
	}
	
    }  //  ami namespace

}  //  tpie namespace

// *********************************************************************
// *                                                                   *
// * Key + Object based Internal Sorter                                *
// *                                                                   *
// *********************************************************************

namespace tpie {

    namespace ami {

	template<class T, class KEY, class CMPR>
	class Internal_Sorter_KObj{

	protected:
	    T* ItemArray;                    // Array that holds original items
	    qsort_item<KEY>* sortItemArray;  // Holds keys to be sorted
	    CMPR *UsrObject;                 // Copy,compare keys
	    TPIE_OS_SIZE_T len;              // length of ItemArray

	public:
	    Internal_Sorter_KObj(CMPR* cmp): ItemArray(NULL), sortItemArray(NULL), UsrObject(cmp), len(0) {
		//  No code in this constructor.
	    }
	    
	    ~Internal_Sorter_KObj(void); //Destructor
    
	    // Allocate array that can hold nItems
	    void allocate(TPIE_OS_SIZE_T nItems);
	    
	    // Sort nItems from input stream and write to output stream
	    err sort(stream<T>* InStr, stream<T>* OutStr, TPIE_OS_SIZE_T nItems);
	    
	    void deallocate(void); //Clean up internal array

	    // Maximum number of Items that can be sorted using memSize bytes
	    TPIE_OS_SIZE_T MaxItemCount(TPIE_OS_SIZE_T memSize);
	    
	    // Memory usage per sort item
	    TPIE_OS_SIZE_T space_per_item();
	    
	    // Fixed memory usage overhead per class instantiation
	    TPIE_OS_SIZE_T space_overhead();
	    
	private:
	    // Prohibit these
	    Internal_Sorter_KObj(const Internal_Sorter_KObj<T,KEY,CMPR>& other);
	    Internal_Sorter_KObj<T,KEY,CMPR> operator=(const Internal_Sorter_KObj<T,KEY,CMPR>& other);
	};
	
	template<class T, class KEY, class CMPR>
	Internal_Sorter_KObj<T, KEY, CMPR>::~Internal_Sorter_KObj(void){

	    //  In case someone forgot to call deallocate()	    
	    if(ItemArray){
		delete [] ItemArray;
		ItemArray=NULL;
	    }
	    
	    if(sortItemArray){
		delete [] sortItemArray;
		sortItemArray=NULL;
	    }
	}

	template<class T, class KEY, class CMPR>
	inline void Internal_Sorter_KObj<T, KEY, CMPR>::allocate(TPIE_OS_SIZE_T nitems){
	    len=nitems;
	    ItemArray = new T[len];
	    sortItemArray = new qsort_item<KEY>[len];
	}

// A helper class to quick sort qsort_item<KEY> types
// given a comparison object for comparing keys
	template<class KEY, class KCMP>
	class QsortKeyCmp {

	private:
	    // Prohibit these.
	    QsortKeyCmp(const QsortKeyCmp<KEY,KCMP>& other);
	    QsortKeyCmp<KEY,KCMP>& operator=(const QsortKeyCmp<KEY,KCMP>& other);
	    
	    KCMP *isLess; //Class with function compare that compares 2 keys    
	    
	public:
	    QsortKeyCmp(KCMP* kcmp) : isLess(kcmp) { };
	    inline int compare(const qsort_item<KEY>& left,
			       const qsort_item<KEY>& right){
		return isLess->compare(left.keyval, right.keyval);
	    }
	};
	
	template<class T, class KEY, class CMPR>
	inline err Internal_Sorter_KObj<T, KEY, CMPR>::sort(stream<T>* InStr,
							    stream<T>* OutStr, TPIE_OS_SIZE_T nItems) {
	    
	    err  ae;
	    T    *next_item;
	    TPIE_OS_SIZE_T i = 0;

	    // Make sure we called allocate earlier
	    if (ItemArray==NULL || sortItemArray==NULL) {
		return NULL_POINTER;
	    }
	    
	    tp_assert ( nItems <= len, "nItems more than interal buffer size.");

	    // Read a memory load out of the input stream one item at a time,
	    for (i = 0; i < nItems; i++) {
		if ((ae=InStr->read_item (&next_item)) != NO_ERROR) {
		    
		    TP_LOG_FATAL_ID ("Internal sort: AMI read error " << ae);
		    
		    return ae;
		}
		
		ItemArray[i] = *next_item;
		UsrObject->copy(&sortItemArray[i].keyval, *next_item);
		sortItemArray[i].source=i;
	    }
	    
	    //Sort the array.
#ifdef TPIE_USE_STL_SORT
	    TP_LOG_DEBUG_ID("calling STL sort for " << nItems << " items");
	    TP_LOG_DEBUG("converting TPIE Key comparison object to STL\n");
	    std::sort(sortItemArray, ItemArray+nItems, 
		      TPIE2STL_cmp<qsort_item<KEY>,QsortKeyCmp<KEY,CMPR> >
		      (QsortKeyCmp<KEY, CMPR>(UsrObject)));
#else
	    QsortKeyCmp<KEY, CMPR> qcmp(UsrObject);
	    TP_LOG_DEBUG("calling quick_sort_obj for " << nItems << " items\n");
	    quick_sort_obj< qsort_item<KEY> > (sortItemArray, nItems, &qcmp);
#endif
  
	    if (InStr==OutStr) { //Do the right thing if we are doing 2x sort
		//Internal sort objects should probably be re-written so that
		//the interface is cleaner and they don't have to worry about I/O
		InStr->truncate(0); //delete original items
		InStr->seek(0); //rewind
	    }
	    
	    //Write sorted array to OutStr
	    for (i = 0; i < nItems; i++) {
		if ((ae = OutStr->write_item(ItemArray[sortItemArray[i].source]))
		    != NO_ERROR) {
		    
		    TP_LOG_FATAL_ID ("Internal Sorter: AMI write error" << ae );

		    return ae;
		}
	    }
	    
	    return NO_ERROR;
	}
	
	template<class T, class KEY, class CMPR>
	inline void Internal_Sorter_KObj<T, KEY, CMPR>::deallocate(void) {
	    
	    len=0;
	    
	    if(ItemArray){
		delete [] ItemArray;
		ItemArray=NULL;
	    }
	    
	    if(sortItemArray){
		delete [] sortItemArray;
		sortItemArray=NULL;
	    }
	}

	template<class T, class KEY, class CMPR>
	inline TPIE_OS_SIZE_T Internal_Sorter_KObj<T, KEY, CMPR>::MaxItemCount(TPIE_OS_SIZE_T memSize) {

	    //Space available for items
	    TPIE_OS_SIZE_T memAvail=memSize-space_overhead();
	    
	    if (memAvail < space_per_item() ){
		return 0; 
	    }
	    else { 
		return memAvail/space_per_item(); 
	    }
	}
	
	
	template<class T, class KEY, class CMPR>
	inline TPIE_OS_SIZE_T Internal_Sorter_KObj<T, KEY, CMPR>::space_overhead(void) { 
	    
	    // Space usage independent of space_per_item
	    // accounts MM_manager space overhead on "new" call
	    return 2*MM_manager.space_overhead();
	}

	template<class T, class KEY, class CMPR>
	inline TPIE_OS_SIZE_T Internal_Sorter_KObj<T, KEY, CMPR>::space_per_item(void) {
	    return sizeof(T) + sizeof(qsort_item<KEY>);
	}

    }  //  ami namespace

}  //  tpie namespace

#endif // _INTERNAL_SORT_H 














































