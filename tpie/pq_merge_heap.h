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

#ifndef _TPIE_PQ_MERGE_HEAP_H_
#define _TPIE_PQ_MERGE_HEAP_H_

#include "ami.h"
#include "tpie_log.h"
#include <cassert>

namespace tpie{

/////////////////////////////////////////////////////////
///
/// \class pq_merge_heap
/// \author Lars Hvam Petersen
///
/// pq_merge_heap
///
/////////////////////////////////////////////////////////
template<typename T, typename Comparator = std::less<T> >
class pq_merge_heap {
	public:
		/////////////////////////////////////////////////////////
		///
		/// Constructor
		///
		/// \param elements Maximum allowed size of the heap
		///
		/////////////////////////////////////////////////////////
		pq_merge_heap(memory_size_type elements);

		/////////////////////////////////////////////////////////
		///
		/// Destructor
		///
		/////////////////////////////////////////////////////////
		~pq_merge_heap();

		/////////////////////////////////////////////////////////
		///
		/// Insert an element into the priority queue
		///
		/// \param x The item
		/// \param run Where it comes from
		///
		/////////////////////////////////////////////////////////
		void push(const T& x, memory_size_type run);

		/////////////////////////////////////////////////////////
		///
		/// Remove the top element from the priority queue
		///
		/////////////////////////////////////////////////////////
		void pop();

		/////////////////////////////////////////////////////////
		///
		/// Remove the top element from the priority queue and 
		/// insert another
		///
		/// \param x The item
		/// \param run Where it comes from
		///
		/////////////////////////////////////////////////////////
		void pop_and_push(const T& x, memory_size_type run);

		/////////////////////////////////////////////////////////
		///
		/// See whats on the top of the priority queue
		///
		/// \return Top element
		///
		/////////////////////////////////////////////////////////
		const T& top() const;

		/////////////////////////////////////////////////////////
		///
		/// Return top element run number
		///
		/// \return Top element run number
		///
		/////////////////////////////////////////////////////////
		memory_size_type top_run() const;

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

	private:
		void fixDown();
		void validate();
		void dump();

		memory_size_type m_size;
		Comparator comp_;

		T* heap;
		memory_size_type* runs;
		memory_size_type maxsize;
};

#include "pq_merge_heap.inl"
}

#endif

