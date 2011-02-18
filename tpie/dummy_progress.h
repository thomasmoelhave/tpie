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
#ifndef __TPIE_DUMMY_PROGRESS__
#define __TPIE_DUMMY_PROGRESS__

#include <tpie/fractional_progress.h>

namespace tpie {

struct dummy_progress_indicator;

struct dummy_fraction_progress {
	inline dummy_fraction_progress(tpie::progress_indicator_base *) {}
	inline dummy_fraction_progress(dummy_progress_indicator *) {}
	inline dummy_fraction_progress & id() {return *this;}

	template <typename T>
	inline dummy_fraction_progress & operator << (const T &) {return *this;}
	inline void init() {}
	inline void done() {}
};

struct dummy_progress_indicator {
	inline dummy_progress_indicator() {}
	inline dummy_progress_indicator(dummy_fraction_progress &, 
					const char *, const char *, const char *, TPIE_OS_OFFSET) {}
	inline void init(TPIE_OS_OFFSET) {}
	inline void step(TPIE_OS_OFFSET) {}
	inline void step() {}
	inline void done() {}
};


template <bool use_progress>
struct progress_types {
	typedef fractional_subindicator sub;
	typedef fractional_progress fp;
	typedef progress_indicator_base base;
};

template <>
struct progress_types<false> {
	typedef dummy_progress_indicator sub;
	typedef dummy_fraction_progress fp;
	typedef dummy_progress_indicator base;
};

}
#endif //__TPIE_DUMMY_PROGRESS__
