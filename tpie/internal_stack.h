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
/// Generic internal stack with known memory requirements.
///////////////////////////////////////////////////////////////////////////
#include <tpie/array.h>
#include <tpie/util.h>
#include <tpie/internal_stack_vector_base.h>
namespace tpie {

/////////////////////////////////////////////////////////
/// \brief A generic internal stack
///
/// \tparam T The type of items storred on the stack
/////////////////////////////////////////////////////////
template <typename T>
class internal_stack: public internal_stack_vector_base<T,internal_stack<T> > {
public:
	typedef internal_stack_vector_base<T,internal_stack<T> > parent_t;
	internal_stack(size_t size=0): parent_t(size){}
	using parent_t::m_elements;
	using parent_t::m_size;
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

};

}
#endif //__TPIE_INTERNAL_STACK_H__
