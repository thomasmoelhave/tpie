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
#ifndef __TPIE_INTERNAL_STACK_VECTOR_BASE_H__
#define __TPIE_INTERNAL_STACK_VECTOR_BASE_H__

///////////////////////////////////////////////////////////////////////////
/// \file internal_stack_vector_base.h
/// Contains a generic base for internal stack and vector with known memory requirements
///////////////////////////////////////////////////////////////////////////
#include <tpie/array.h>
#include <tpie/util.h>

namespace tpie {
	
/////////////////////////////////////////////////////////
/// \brief A base class for generic internal fixed size stack and vector
///
/// \tparam T The type of items storred in the container
/// \tparam child_t The subtype of the class
/////////////////////////////////////////////////////////
template <typename T, typename child_t>
class internal_stack_vector_base: public linear_memory_base<child_t> {
protected:
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
	static double memory_overhead() {return array<T>::memory_overhead() - sizeof(array<T>) + sizeof(child_t);}

	/////////////////////////////////////////////////////////
	/// \brief Construct the data structure
	///
	/// \param size The maximal number of items allowed in the container at any one time
	/////////////////////////////////////////////////////////
	internal_stack_vector_base(size_t size=0): m_size(0) {m_elements.resize(size);}

	/////////////////////////////////////////////////////////
	/// \brief Resize the data structure, all data is lost
	///
	/// \param size The number of pushes supported between calles to
	/////////////////////////////////////////////////////////
	void resize(size_t size=0) {m_elements.resize(size); m_size=0;}
	
	/////////////////////////////////////////////////////////
	/// \brief Check if the data structure is empty
	/// \return true if the data structure is empty otherwize false
	/////////////////////////////////////////////////////////
	inline bool empty(){return m_size==0;}

	/////////////////////////////////////////////////////////
	/// \brief Return the number of elements in the data structure.
	/// \return The number of elements in the container
	/////////////////////////////////////////////////////////
	inline size_t size(){ return m_size;}
	
	/////////////////////////////////////////////////////////
	/// \brief Clear the data structure of all elements
	/////////////////////////////////////////////////////////
	inline void clear(){m_size=0;}
};

}
#endif //__TPIE_INTERNAL_STACK_VECTOR_BASE_H__
