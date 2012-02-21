// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, The TPIE development team
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

#ifndef __TPIE_PIPELINING_STDIO_H__
#define __TPIE_PIPELINING_STDIO_H__

#include <tpie/pipelining/core.h>
#include <tpie/pipelining/factory_helpers.h>
#include <cstdio>

namespace tpie {

namespace pipelining {

template <typename dest_t>
struct scanf_ints_t : public pipe_segment {
	typedef int item_type;

	inline scanf_ints_t(const dest_t & dest) : dest(dest) {
	}

	inline void operator()() {
		dest.begin();
		int in;
		while (scanf("%d", &in) == 1) {
			dest.push(in);
		}
		dest.end();
	}


	const pipe_segment * get_next() const {
		return &dest;
	}
private:
	dest_t dest;
};

struct printf_ints_t : public pipe_segment {
	typedef int item_type;

	inline printf_ints_t() {
	}

	inline void begin() { }
	inline void end() { }

	inline void push(item_type i) {
		printf("%d\n", i);
	}

	const pipe_segment * get_next() const { return 0; }
};

pipe_begin<factory_0<scanf_ints_t> >
inline scanf_ints() {
	return factory_0<scanf_ints_t>();
}

termfactory_0<printf_ints_t>
inline printf_ints() {
	return termfactory_0<printf_ints_t>();
}

}

}

#endif
