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
#ifndef __TPIE_INTERNAL_STACK_H__
#define __TPIE_INTERNAL_STACK_H__

///////////////////////////////////////////////////////////////////////////
/// \file internal_stack.h
/// Contains a generic internal stack with known memory requirements
///////////////////////////////////////////////////////////////////////////
#include <tpie/array.h>
#include <tpie/util.h>

namespace tpie {

/////////////////////////////////////////////////////////
/// \brief A generic internal stack
///
/// \tparam T The type of items storred on the stack
/////////////////////////////////////////////////////////
template <typename T>
class internal_stack: public linear_memory_base<internal_stack<T> > {
	array<T> m_elements;
	size_t m_size;
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
	static double memory_overhead() {return array<T>::memory_overhead() - sizeof(array<T>) + sizeof(internal_stack);}

	/////////////////////////////////////////////////////////
	/// \brief Construct a stack
	///
	/// \param size The maximal number of items allowed on the stack at any one time
	/////////////////////////////////////////////////////////
	internal_stack(size_t size=0): m_size(0) {m_elements.resize(size);}

	/////////////////////////////////////////////////////////
	/// \brief Resize the stack, all data is lost
	///
	/// \param size The number of pushes supported between calles to
	/////////////////////////////////////////////////////////
	void resize(size_t size=0) {m_elements.resize(size); m_size=0;}
	
	/////////////////////////////////////////////////////////
	/// \brief Return the top most element on the stack
	/////////////////////////////////////////////////////////
	inline T & top() {return m_elements[m_size-1];}

	/////////////////////////////////////////////////////////
	/// \brief Add an element to the top of the stack
	///
	/// \param val The element to add
	/////////////////////////////////////////////////////////
	inline void push(const T & val){m_elements[m_size++] = val;}

	/////////////////////////////////////////////////////////
	/// \brief Remove the top most element from the stack
	/////////////////////////////////////////////////////////
	inline void pop(){--m_size;}

	/////////////////////////////////////////////////////////
	/// \brief Check if the stacke is empty
	/// \return true if the stack is empty otherwize false
	/////////////////////////////////////////////////////////
	inline bool empty(){return m_size==0;}

	/////////////////////////////////////////////////////////
	/// \brief Return the number of elements on the stack.
	/// \return The number of elements on the stack
	/////////////////////////////////////////////////////////
	inline size_t size(){ return m_size;}
	
	/////////////////////////////////////////////////////////
	/// \brief Clear the stack of all elements
	/////////////////////////////////////////////////////////
	inline void clear(){m_size=0;}
};

}
#endif //__TPIE_INTERNAL_STACK_H__
