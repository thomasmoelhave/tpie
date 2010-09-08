// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
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
#include <tpie/array.h>
#include <algorithm>
#include <tpie/util.h>
namespace tpie {

/////////////////////////////////////////////////////////
/// \file internal_priority_queue.h
///
/// \brief Simple heap based priority queue implementation
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
/// \class internal_priority_queue
/// \author Lars Hvam Petersen, Jakob Truelsen
/// 
/// Standard binary internal heap.
/////////////////////////////////////////////////////////
template <typename T, typename comp_t = std::less<T> >
class internal_priority_queue: public linear_memory_base< internal_priority_queue<T, comp_t> > {
public:

    /////////////////////////////////////////////////////////
    /// Constructor
    ///
    /// \param max_size Maximum size of queue
    /////////////////////////////////////////////////////////
    internal_priority_queue(size_type max_size, comp_t c=comp_t()): pq(max_size), sz(0), comp(c) {}
    //pq_internal_heap(T* arr, TPIE_OS_SIZE_T length) pq(arr, length), sz(length) {}
  
    /////////////////////////////////////////////////////////
    /// Return true if queue is empty otherwise false
    ///
    /// \return True if the queue is empty
    /////////////////////////////////////////////////////////
    bool empty() const {return sz == 0;}

    /////////////////////////////////////////////////////////
    /// Returns the size of the queue
    ///
    /// \return Queue size
    /////////////////////////////////////////////////////////
    inline size_type size() const {return sz;}

    /////////////////////////////////////////////////////////
    /// Insert an element into the priority queue
    ///
    /// \param v The element that should be inserted
    /////////////////////////////////////////////////////////
    inline void push(const T & v) { 
		pq[sz++] = v; 
		std::push_heap(pq.begin(), pq.find(sz), comp);
    }

    /////////////////////////////////////////////////////////
    /// Remove the minimum element from heap
    /////////////////////////////////////////////////////////
    inline void pop() { 
		std::pop_heap(pq.begin(), pq.find(sz), comp);
		--sz;
    }

    /////////////////////////////////////////////////////////
    /// Return the minimum element
    ///
    /// \return The minimum element
    /////////////////////////////////////////////////////////
    inline const T & top() const {return pq[0];}
	

	/////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_coefficient()
	/// \copydetails linear_memory_structure_doc::memory_coefficient()
	/////////////////////////////////////////////////////////
	inline static double memory_coefficient() {
		return tpie::array<T>::memory_coefficient();
	}

	/////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_overhead()
	/// \copydetails linear_memory_structure_doc::memory_overhead()
	/////////////////////////////////////////////////////////
	inline static double memory_overhead() {
		return tpie::array<T>::memory_overhead() - sizeof(tpie::array<T>) + sizeof(internal_priority_queue);
	}

	/////////////////////////////////////////////////////////
    /// \brief Return the underlaying array 
    ///
	/// Make sure you know what you are doing
	///
    /// \return The underlaying array
    /////////////////////////////////////////////////////////
	tpie::array<T> & get_array() {
		return pq;
	}

	/////////////////////////////////////////////////////////
	/// \brief Clear the structure of all elements
	/////////////////////////////////////////////////////////
	inline void clear() {sz=0;}
private:	
	template <typename TT>
	struct binary_argument_swap {
		TT i;
		binary_argument_swap(TT & _): i(_) {}
		bool operator()(const T& a, const T&b) const {return i(b,a);}
	};

	tpie::array<T> pq; 
    size_type sz;
	binary_argument_swap<comp_t> comp;
};

}  //  tpie namespace
#endif
