// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014, The TPIE development team
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
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE. If not, see <http://www.gnu.org/licenses/>

#ifndef __TPIE_PIPELINING_RUN_CONTAINER_H__
#define __TPIE_PIPELINING_RUN_CONTAINER_H__

#include <tpie/tpie_assert.h>
#include <tpie/array.h>

namespace tpie {

namespace bits {

template <typename T>
class run_container {
public:
	typedef typename array<T>::iterator iterator;

	inline run_container(memory_size_type maxSize)
	: m_data(maxSize)
	, m_size(0)
	{}

	memory_size_type size() const {
		return m_size;
	}

	void push(const T & e) {
		m_data[m_size++] = e;
	}

	iterator begin() {
		return m_data.begin();
	}

	iterator end() {
		return m_data.end()+m_size;
	}

	void clear() {
		m_size = 0;
	}

	bool empty() {
		return m_size == 0;
	}
private:
	array<T> m_data;
	memory_size_type m_size;
};

} // namespace bits

} // namespace tpie

#endif // __TPIE_PIPELINING_RUN_CONTAINER_H__