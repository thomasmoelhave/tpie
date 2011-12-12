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

#ifndef __TPIE_PIPELINING_HELPERS_H__
#define __TPIE_PIPELINING_HELPERS_H__

#include <iostream>
#include <tpie/pipelining/core.h>
#include <tpie/pipelining/factory_helpers.h>

namespace tpie {

template <typename dest_t>
struct ostream_logger_t {
	typedef typename dest_t::item_type item_type;

	inline ostream_logger_t(const dest_t & dest, std::ostream & log) : dest(dest), log(log) {
	}
	inline void begin() {
		begun = true;
		dest.begin();
	}
	inline void end() {
		ended = true;
		dest.end();
	}
	inline void push(const item_type & item) {
		if (!begun) {
			log << "WARNING: push() called before begin(). Calling begin on rest of pipeline." << std::endl;
			begin();
		}
		if (ended) {
			log << "WARNING: push() called after end()." << std::endl;
			ended = false;
		}
		log << "pushing " << item << std::endl;
		dest.push(item);
	}
private:
	dest_t dest;
	std::ostream & log;
	bool begun;
	bool ended;
};

inline generate<factory_1<ostream_logger_t, std::ostream &> >
cout_logger() {
	return factory_1<ostream_logger_t, std::ostream &>(std::cout);
}

}

#endif
