// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2010, The TPIE development team
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
#ifndef __TPIE_SIMPLEQUEUE_H_
#define __TPIE_SIMPLEQUEUE_H_

///////////////////////////////////////////////////////////////////////////
/// \file internal_queue.h
/// Contains a generic internal queue with known memory requirements
///////////////////////////////////////////////////////////////////////////
#include <tpie/array.h>
#include <tpie/util.h>

namespace tpie {

/////////////////////////////////////////////////////////
/// \brief A generic internal queue
///
/// The queue supports a fixed number of pushes between
/// calls to clear, the number of pushes is given as an argument
/// to the constructructor or to resize. This means that the
/// queue does NOT imprement a ring buffer.
///
/// \tparam T The type of items storred in the queue
/////////////////////////////////////////////////////////
template <typename T>
class internal_queue: public linear_memory_base<internal_queue<T> > {
	array<T> m_elements;
	size_t m_first, m_last;
public:
	/////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_coefficient()
	/// \copydetails linear_memory_structure_doc::memory_coefficient()
	/////////////////////////////////////////////////////////
	static double memory_coefficient() {return array<T>::memory_coefficient();}

	/////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_overhead()
	/// \copydetails linear_memory_structure_doc::memory_overhead()
	/////////////////////////////////////////////////////////
	static double memory_overhead() {return array<T>::memory_overhead() - sizeof(array<T>) + sizeof(internal_queue);}

	/////////////////////////////////////////////////////////
	/// \brief Construct an queue
	///
	/// \param size The number of pushes supported between calles to
	/////////////////////////////////////////////////////////
	internal_queue(size_t size=0): m_first(0), m_last(0) {m_elements.resize(size);}

	/////////////////////////////////////////////////////////
	/// \brief Resize the queue, all data is lost
	///
	/// \param size The number of pushes supported between calles to
	/////////////////////////////////////////////////////////
	void resize(size_t size=0) {m_elements.resize(size); m_first = m_last = 0;}
	
	/////////////////////////////////////////////////////////
	/// \brief Return the item that has been in the queue for the longest time
	/////////////////////////////////////////////////////////
	inline T & front() {return m_elements[m_first];}

	/////////////////////////////////////////////////////////
	/// \brief Return the last item pushed to the queue
	/////////////////////////////////////////////////////////
	inline T & back() {return m_elements[m_last-1];}

	/////////////////////////////////////////////////////////
	/// \brief Add an element to the front of the queue
	///
	/// \param val The element to add
	/////////////////////////////////////////////////////////
	inline void push(const T & val){m_elements[m_last++] = val;}

	/////////////////////////////////////////////////////////
	/// \brief Remove an element from the back of the queue
	/////////////////////////////////////////////////////////
	inline void pop(){++m_first;}

	/////////////////////////////////////////////////////////
	/// \brief Check if the queue is empty
	/// \return true if the queue is empty otherwize false
	/////////////////////////////////////////////////////////
	inline bool empty(){return m_first == m_last;}

	/////////////////////////////////////////////////////////
	/// \brief Return the number of elements in the queue.
	/// \return The number of elements in the queue
	/////////////////////////////////////////////////////////
	inline size_t size(){ return m_last - m_first;}
	
	/////////////////////////////////////////////////////////
	/// \brief Clear the queue of all elements
	///
	/// After this call the queue again supports the number of
	/// pushes given in the constructor or resize.
	/////////////////////////////////////////////////////////
	inline void clear(){m_first = m_last =0;}
};

}
#endif //__TERRASTREAM_SIMPLEQUEUE_H_
