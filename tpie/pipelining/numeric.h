// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, The TPIE development team
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

#ifndef __TPIE_PIPELINING_NUMERIC_H__
#define __TPIE_PIPELINING_NUMERIC_H__

#include <iostream>
#include <tpie/pipelining/core.h>
#include <tpie/pipelining/factory_helpers.h>

namespace tpie {

namespace pipelining {

template <typename dest_t>
struct linear_t : public pipe_segment {
	typedef typename dest_t::item_type item_type;

	inline linear_t(const dest_t & dest, item_type factor, item_type term) : dest(dest), factor(factor), term(term) {
	}
	inline void begin() { }
	inline void end() { }
	inline void push(const item_type & item) {
		dest.push(item*factor+term);
	}

	const pipe_segment * get_next() const {
		return &dest;
	}
private:
	dest_t dest;
	item_type factor;
	item_type term;
};

template <typename T>
inline pipe_middle<factory_2<linear_t, T, T> >
linear(T factor, T term) {
	return factory_2<linear_t, T, T>(factor, term);
}

}

}

#endif
