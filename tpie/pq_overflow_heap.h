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

#ifndef _TPIE_PQ_OVERFLOW_HEAP_H_
#define _TPIE_PQ_OVERFLOW_HEAP_H_

#include "pq_internal_heap.h"

namespace tpie {

    namespace ami {

/////////////////////////////////////////////////////////
///
///  \class pq_overflow_heap
///  \author Lars Hvam Petersen
///
///  Overflow Priority Queue, based on a simple Heap
///
/////////////////////////////////////////////////////////
template<typename T, typename Comparator = std::less<T> >
class pq_overflow_heap {
public:
    /////////////////////////////////////////////////////////
    ///
    /// Constructor
    ///
    /// \param maxsize Maximal size of queue
    ///
    /////////////////////////////////////////////////////////
    pq_overflow_heap(memory_size_type maxsize);

    /////////////////////////////////////////////////////////
    ///
    /// Destructor
    ///
    /////////////////////////////////////////////////////////
    ~pq_overflow_heap();

    /////////////////////////////////////////////////////////
    ///
    /// Insert an element into the priority queue
    ///
    /// \param x The item
    ///
    /////////////////////////////////////////////////////////
    void push(const T& x);

    /////////////////////////////////////////////////////////
    ///
    /// Remove the top element from the priority queue
    ///
    /////////////////////////////////////////////////////////
    void pop();

    /////////////////////////////////////////////////////////
    ///
    /// See whats on the top of the priority queue
    ///
    /// \return Top element
    ///
    /////////////////////////////////////////////////////////
    const T& top();

    /////////////////////////////////////////////////////////
    ///
    /// Returns the size of the queue
    ///
    /// \return Queue size
    ///
    /////////////////////////////////////////////////////////
    memory_size_type size() const;

    /////////////////////////////////////////////////////////
    ///
    /// Return true if queue is empty otherwise false
    ///
    /// \return Boolean - empty or not
    ///
    /////////////////////////////////////////////////////////
    bool empty() const;

    /////////////////////////////////////////////////////////
    ///
    /// The factor of the size, total, which is returned 
    /// sorted 
    ///
    /////////////////////////////////////////////////////////
    static const double sorted_factor;
		
    /////////////////////////////////////////////////////////
    ///
    /// Returns whether the overflow heap is full or not
    ///
    /// \return Boolean - full or not
    ///
    /////////////////////////////////////////////////////////
    bool full() const;

    /////////////////////////////////////////////////////////
    ///
    /// Sorts the underlying array and returns a pointer to it, this operation invalidades the heap.
    ///
    /// \return A pointer to the sorted underlying array
    ///
    /////////////////////////////////////////////////////////
    T* sorted_array();

    /////////////////////////////////////////////////////////
    ///
    /// Return size of sorted array
    ///
    /// \return Size
    ///
    /////////////////////////////////////////////////////////
    memory_size_type sorted_size() const;

    /////////////////////////////////////////////////////////
    ///
    /// Remove all elements from queue 
    ///
    /////////////////////////////////////////////////////////
    void sorted_pop();

private:
    Comparator comp_;
    pq_internal_heap<T, Comparator>* h;
    memory_size_type maxsize;
    T dummy;
};
	
	template<typename T, typename Comparator>
	const double pq_overflow_heap<T,Comparator>::sorted_factor = 1.0;

#include "pq_overflow_heap.inl"

    }  //  ami namespace

}  //  tpie namespace

#endif
