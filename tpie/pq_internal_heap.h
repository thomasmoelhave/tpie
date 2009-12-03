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

#ifndef __TPIE_AMI_PQ_INTERNAL_HEAP_H__
#define __TPIE_AMI_PQ_INTERNAL_HEAP_H__

namespace tpie {

    namespace ami {

/////////////////////////////////////////////////////////
///
/// \class Heap
/// \author Lars Hvam Petersen
/// 
/// Standard binary internal heap.
///
/////////////////////////////////////////////////////////

template <typename T, typename Comparator = std::less<T> >
class pq_internal_heap {
private:
    Comparator comp_;

public:
    /////////////////////////////////////////////////////////
    ///
    /// Constructor
    ///
    /// \param max_size Maximum size of queue
    ///
    /////////////////////////////////////////////////////////
    pq_internal_heap(TPIE_OS_SIZE_T max_size, Comparator c=Comparator()):
		comp_(c) { 
	pq = new T[max_size]; 
	sz = 0; 
    }

    pq_internal_heap(T* arr, TPIE_OS_SIZE_T length) {
	pq = arr;
	sz = length;
    }
  
    /////////////////////////////////////////////////////////
    ///
    /// Destructor
    ///
    /////////////////////////////////////////////////////////
    ~pq_internal_heap() { delete[] pq; } 
  
    /////////////////////////////////////////////////////////
    ///
    /// Return true if queue is empty otherwise false
    ///
    /// \return Boolean - empty or not
    ///
    /////////////////////////////////////////////////////////
    inline bool empty() const { return sz == 0; }

    /////////////////////////////////////////////////////////
    ///
    /// Returns the size of the queue
    ///
    /// \return Queue size
    ///
    /////////////////////////////////////////////////////////
    inline TPIE_OS_SIZE_T size() const{
	return sz;
    }

    /////////////////////////////////////////////////////////
    ///
    /// Insert an element into the heap 
    ///
    /// \param v The element that should be inserted
    ///
    /////////////////////////////////////////////////////////
    inline void insert(const T & v) { 
	pq[sz++] = v; 
	bubbleUp(sz-1);
    }

    /////////////////////////////////////////////////////////
    ///
    /// Remove the minimal element from heap
    ///
    /// \return Minimal element
    ///
    /////////////////////////////////////////////////////////
    inline T & delmin() { 
	swap(pq[0], pq[--sz]);
	bubbleDown(); 
	return pq[sz];
    }

	
    inline T delminAndInsert(T item) { 
		swap(pq[0], item);
		bubbleDown(); 
		return item;
    }

    /////////////////////////////////////////////////////////
    ///
    /// Peek the minimal element
    ///
    /// \return Minimal element
    ///
    /////////////////////////////////////////////////////////
    const T & peekmin() const {
	return pq[0]; 
    }
	
    /////////////////////////////////////////////////////////
    ///
    /// Set the size 
    ///
    /// \param ne Size
    ///
    /////////////////////////////////////////////////////////
    void set_size(TPIE_OS_SIZE_T ne) {
	sz = ne; 
    }
	
    /////////////////////////////////////////////////////////
    ///
    /// Returns a pointer to the underlaying array 
    ///
    /// \return Array
    ///
    /////////////////////////////////////////////////////////
    T* get_arr() { 
	return pq; 
    }
	
private:

    inline TPIE_OS_SIZE_T left_child(TPIE_OS_SIZE_T k) {
	return 2*k+1;
    }

    inline TPIE_OS_SIZE_T right_child(TPIE_OS_SIZE_T k){
	return 2*k+2;
    }

    inline TPIE_OS_SIZE_T parent(TPIE_OS_SIZE_T k){
	return (k-1)/2;
    }

    void bubbleDown() { 
	TPIE_OS_SIZE_T k=0;
	TPIE_OS_SIZE_T j;
	while((j=left_child(k)) < sz) {
	    if(j < sz-1 && comp_(pq[j+1], pq[j])) j++; // compare, pq[j] > pq[j+1]
	    if(! comp_(pq[j], pq[k]) ) break; // compare, pq[k] > pq[j]
	    swap(pq[k], pq[j]); 
	    k = j;
	}
    }
  
	//  Parameter has to be signed!
    void bubbleUp(TPIE_OS_SSIZE_T k) {
	TPIE_OS_SSIZE_T j;
	while(k > 0 && comp_(pq[k], pq[(j=parent(k))])) { // compare, pq[k/2] > pq[k]
	    swap(pq[k], pq[j]);
	    k = j; 
	}
    }
		
    T *pq; 
    TPIE_OS_SIZE_T sz;
};

    }  // ami namespace

}  //  tpie namespace

#endif
