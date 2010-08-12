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
#ifndef __TPIE_SIMPLEQUEUE_H_
#define __TPIE_SIMPLEQUEUE_H_
#include <tpie/array.h>

namespace tpie {

template <typename T>
class simplequeue{
	array<T> m_elements;
	size_t m_first, m_last;
public:
	static size_t memory_required(size_t size) {
		return sizeof(simplequeue) + array<T>::memory_required(size) - sizeof(array<T>);
	}
	simplequeue(size_t size=0): m_first(0), m_last(0) {m_elements.resize(size);}
	void resize(size_t size=0) {m_elements.resize(size); m_first = m_last = 0;}
	inline T & front() {return m_elements[m_first];}
	inline T & back() {return m_elements[m_last-1];}
	inline void push(const T & val){m_elements[m_last++] = val;}
	inline void pop(){++m_first;}
	inline bool empty(){return m_first == m_last;}
	inline size_t size(){ return m_last - m_first;}
	inline void clear(){m_first = m_last =0;}
};

}
#endif __TERRASTREAM_SIMPLEQUEUE_H_
