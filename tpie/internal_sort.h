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

#ifndef _INTERNAL_SORT_H
#define _INTERNAL_SORT_H

///////////////////////////////////////////////////////////////////////////
/// \file internal_sort.h
/// Provides base class Internal_Sorter_Base for internal sorter objects and
/// two subclass implementations Internal_Sorter_Op and Internal_Sorter_Obj.
/// Both implementations rely on quicksort variants quick_sort_op() and 
/// quick_sort_obj(), resp.
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <algorithm>
#include <tpie/comparator.h> //to convert TPIE comparisons to STL

namespace tpie {
namespace ami {

	//A simple class that facilitates doing key sorting followed 
	//by in-memory permuting to sort items in-memory. This is 
	//particularly useful when key size is much smaller than 
	//item size. Note that using this requires that the class Key
	//have the comparison operators defined appropriately.
	template<class Key>
		class qsort_item {
			public:
				Key keyval;
				TPIE_OS_SIZE_T source;

				friend int operator==(const qsort_item &x, const qsort_item &y)
				{return  (x.keyval ==  y.keyval);}

				friend int operator!=(const qsort_item &x, const qsort_item &y)
				{return  (x.keyval !=  y.keyval);}    

				friend int operator<=(const qsort_item &x, const qsort_item &y)
				{return  (x.keyval <=  y.keyval);}

				friend int operator>=(const qsort_item &x, const qsort_item &y)
				{return  (x.keyval >=  y.keyval);}

				friend int operator<(const qsort_item &x, const qsort_item &y)
				{return  (x.keyval <  y.keyval);}

				friend int operator>(const qsort_item &x, const qsort_item &y)
				{return  (x.keyval >  y.keyval);}

		};
		

  ///////////////////////////////////////////////////////////////////////////
  /// The base class for internal sorters. 
  /// This class does not have a sort() function, so it cannot be used directly.
  ///////////////////////////////////////////////////////////////////////////
	template<class T>
	class Internal_Sorter_Base {
	    
	protected:
	    /** Array that holds items to be sorted */
	    T* ItemArray;        
	    /** length of ItemArray */
	    TPIE_OS_SIZE_T len;  
	    
	public:
	    ///////////////////////////////////////////////////////////////////////////
	    ///  Empty constructor.
	    ///////////////////////////////////////////////////////////////////////////
	    Internal_Sorter_Base(void): ItemArray(NULL), len(0) {
		//  No code in this constructor.
	    };
	    
      ///////////////////////////////////////////////////////////////////////////
      ///  Destructor.
      ///////////////////////////////////////////////////////////////////////////
	    virtual ~Internal_Sorter_Base(void); 
    
      ///////////////////////////////////////////////////////////////////////////
      /// Allocate ItemArray as array that can hold \p nItems.
      ///////////////////////////////////////////////////////////////////////////
	    void allocate(TPIE_OS_SIZE_T nItems);
	    
      ///////////////////////////////////////////////////////////////////////////
      /// Clean up internal array ItemArray.
      ///////////////////////////////////////////////////////////////////////////
      void deallocate(void); 
	    
	    ///////////////////////////////////////////////////////////////////////////
	    /// Returns maximum number of items that can be sorted using \p memSize bytes.
	    ///////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T MaxItemCount(TPIE_OS_SIZE_T memSize);
	    
      ///////////////////////////////////////////////////////////////////////////
      /// Returns memory usage in bytes per sort item.
      ///////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T space_per_item();
	    
      ///////////////////////////////////////////////////////////////////////////
      /// Returns fixed memory usage overhead in bytes per class instantiation.
      ///////////////////////////////////////////////////////////////////////////
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
	
  ///////////////////////////////////////////////////////////////////////////
  /// Comparision operator based Internal_Sorter_base subclass implementation; uses 
  /// quick_sort_op().
  ///////////////////////////////////////////////////////////////////////////
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
	
  ///////////////////////////////////////////////////////////////////////////
  /// Reads nItems sequentially from InStr, starting at the current file
  /// position; writes the sorted output to OutStr, starting from the current
  /// file position.
  ///////////////////////////////////////////////////////////////////////////
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
	    TP_LOG_DEBUG_ID("calling STL sort for " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(nItems) << " items");
	    std::sort(ItemArray, ItemArray+nItems);
	    TP_LOG_DEBUG("calling quick_sort_op for " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(nItems) << " items\n");
	    
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

  ///////////////////////////////////////////////////////////////////////////
  /// Comparision object based Internal_Sorter_base subclass implementation; uses 
  /// quick_sort_obj().
  ///////////////////////////////////////////////////////////////////////////
	template<class T, class CMPR>
	class Internal_Sorter_Obj: public Internal_Sorter_Base<T>{

	protected:
	    using Internal_Sorter_Base<T>::ItemArray;
	    using Internal_Sorter_Base<T>::len;
	    /** Comparison object used for sorting */
	    CMPR *cmp_o;
	    
	public:
      ///////////////////////////////////////////////////////////////////////////
      /// Empty constructor.
      ///////////////////////////////////////////////////////////////////////////
      Internal_Sorter_Obj(CMPR* cmp) :cmp_o(cmp) {};

      ///////////////////////////////////////////////////////////////////////////
      /// Empty destructor.
      ///////////////////////////////////////////////////////////////////////////
      ~Internal_Sorter_Obj(){};
	    
	    using Internal_Sorter_Base<T>::space_overhead;
	    
	    //Sort nItems from input stream and write to output stream
	    err sort(stream<T>* InStr, stream<T>* OutStr, TPIE_OS_SIZE_T nItems);
	    
	private:
	    // Prohibit these
	    Internal_Sorter_Obj(const Internal_Sorter_Obj<T,CMPR>& other);
	    Internal_Sorter_Obj<T,CMPR> operator=(const Internal_Sorter_Obj<T,CMPR>& other);
	};
	
  ///////////////////////////////////////////////////////////////////////////
  /// Reads nItems sequentially from InStr, starting at the current file
  /// position; writes the sorted output to OutStr, starting from the current
  /// file position.
  ///////////////////////////////////////////////////////////////////////////
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
	    TP_LOG_DEBUG_ID("calling STL sort for " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(nItems) << " items");
	    TP_LOG_DEBUG("converting TPIE comparison object to STL\n");
	    std::sort(ItemArray, ItemArray+nItems, TPIE2STL_cmp<T,CMPR>(cmp_o));
	    
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
	

  ////////////////////////////////////////////////////////////////////////
  /// Key + Object based Internal Sorter; used by key_sort() routines.
  ////////////////////////////////////////////////////////////////////////
  template<class T, class KEY, class CMPR>
  class Internal_Sorter_KObj : public Internal_Sorter_Base<T> {

	protected:
	    /** Array that holds original items */
	    T* ItemArray;                    
	    /** Holds keys to be sorted */
	    qsort_item<KEY>* sortItemArray;  
	    /** Copy,compare keys */ 
	    CMPR *UsrObject;              
	    /** length of ItemArray */
	    TPIE_OS_SIZE_T len;

	public:
      ///////////////////////////////////////////////////////////////////////////
      ///  Empty constructor.
      ///////////////////////////////////////////////////////////////////////////
	    Internal_Sorter_KObj(CMPR* cmp): ItemArray(NULL), sortItemArray(NULL), UsrObject(cmp), len(0) {
		//  No code in this constructor.
	    }
	    
      //////////////////////////////////////////////////////////////////////////
      ///  Destructor calling deallocate.
      //////////////////////////////////////////////////////////////////////////
	    ~Internal_Sorter_KObj(void); //Destructor
    
      //////////////////////////////////////////////////////////////////////////
      /// Allocate array that can hold nItems.
      //////////////////////////////////////////////////////////////////////////
	    void allocate(TPIE_OS_SIZE_T nItems);
	    
      //////////////////////////////////////////////////////////////////////////
      /// Sort nItems from input stream and write to output stream.
      //////////////////////////////////////////////////////////////////////////
	    err sort(stream<T>* InStr, stream<T>* OutStr, TPIE_OS_SIZE_T nItems);
	    
      //////////////////////////////////////////////////////////////////////////
      /// Clean up internal array.
      //////////////////////////////////////////////////////////////////////////
      void deallocate(void); 

      //////////////////////////////////////////////////////////////////////////
      /// Returns maximum number of items that can be sorted using \p memSize bytes.
      //////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T MaxItemCount(TPIE_OS_SIZE_T memSize);
	    
      //////////////////////////////////////////////////////////////////////////
      /// Returns memory usage in bytes per sort item.
      //////////////////////////////////////////////////////////////////////////
	    TPIE_OS_SIZE_T space_per_item();
	    
      //////////////////////////////////////////////////////////////////////////
      /// Returns fixed memory usage overhead in bytes per class instantiation.
      //////////////////////////////////////////////////////////////////////////
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

  ///////////////////////////////////////////////////////////////////////////
  /// A helper class to quick sort qsort_item types
  /// given a comparison object for comparing keys.
  ///////////////////////////////////////////////////////////////////////////
  template<class KEY, class KCMP>
  class QsortKeyCmp {

	private:
	    // Prohibit these.
	    QsortKeyCmp(const QsortKeyCmp<KEY,KCMP>& other);
	    QsortKeyCmp<KEY,KCMP>& operator=(const QsortKeyCmp<KEY,KCMP>& other);
	    
	    KCMP *isLess; //Class with function compare that compares 2 keys    
	    
	public:
      ///////////////////////////////////////////////////////////////////////////
      ///  Constructor.
      ///////////////////////////////////////////////////////////////////////////
      QsortKeyCmp(KCMP* kcmp) : isLess(kcmp) { };

      ///////////////////////////////////////////////////////////////////////////
      /// Comparision method;
      /// returns -1, 0, or +1 to indicate that <tt>left<right</tt>, <tt>left==right</tt>, or
      /// <tt>left>right</tt> respectively.
      ///////////////////////////////////////////////////////////////////////////
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
	    TP_LOG_DEBUG_ID("calling STL sort for " << static_cast<TPIE_OS_OUTPUT_SIZE_T>(nItems) << " items");
	    TP_LOG_DEBUG("converting TPIE Key comparison object to STL\n");
		QsortKeyCmp<KEY, CMPR> kc(UsrObject);
		TPIE2STL_cmp<qsort_item<KEY>,QsortKeyCmp<KEY,CMPR> > stlcomp(&kc);
	    std::sort(
				sortItemArray, sortItemArray+nItems/*, stlcomp */
			  );
  
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














































