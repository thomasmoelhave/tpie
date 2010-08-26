// -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team
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

#ifndef __TPIE_UTIL_H__
#define __TPIE_UTIL_H__

#include <tpie/portability.h>
#include <cmath>
namespace tpie {

template <typename T>
inline void unused(const T & x) {(void)x;}

typedef TPIE_OS_OFFSET offset_type;
typedef TPIE_OS_SIZE_T size_type;
typedef TPIE_OS_SSIZE_T ssize_type;

template <typename child_t> 
struct linear_memory_base {
	inline static offset_type memory_usage(offset_type size) {
		return ceil( size * child_t::memory_coefficient() + child_t::memory_overhead() );
	}

	inline static size_type memory_fits(size_type memory) {
		return floor( (memory - child_t::memory_overhead() - 1) / child_t::memory_coefficient() );
	}
};

}

#endif //__TPIE_UTIL_H__
