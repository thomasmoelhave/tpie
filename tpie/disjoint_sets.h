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

#ifndef __TPIE_DISJOINT_SETS__
#define __TPIE_DISJOINT_SETS__
#include <tpie/array.h>
#include <tpie/unused.h>
#include <tpie/util.h>

namespace tpie {


template <typename value_t> 
class disjoint_sets: public linear_memory_base< disjoint_sets<value_t> > {
private:
	array<value_t> m_elements;
	value_t m_unused;
	size_type m_size;
public:
	disjoint_sets(size_type n, value_t u = default_unused<value_t>::v()): m_elements(n, u), m_unused(u), m_size(0) {}

	static double memory_coefficient() {
		return array<value_t>::memory_coefficient();
	}

	static double memory_overhead() {
		return array<value_t>::memory_overhead() + sizeof(disjoint_sets) - sizeof(array<value_t>);
	}
	
	inline void make_set(value_t element) {m_elements[element] = element; ++m_size;}
	inline bool is_set(value_t element) {return m_elements[element] != m_unused;}

	inline value_t link(value_t a, value_t b) {
		if (a == b) return a;
		--m_size;
		m_elements[b] = a;
		return a;
	}

	inline value_t find_set(value_t t) {
		while (true) {
			value_t x = m_elements[m_elements[t]];
			if (x == t) return t;
			m_elements[t] = x;
			t = x;
		}
	}

	inline value_t union_set(value_t a, value_t b) {return link(find_set(a), find_set(b));}
	inline size_t count_sets() {return m_size;}
};

}
#endif //__TPIE_DISJOINT_SETS__
