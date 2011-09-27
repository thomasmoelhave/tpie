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
#ifndef __COMMON_H__
#define __COMMON_H__

#include "../app_config.h"
#include <tpie/memory.h>
#include <tpie/util.h>
#include <iostream>
#include <boost/cstdint.hpp>
#include <tpie/tpie.h>

struct bit_permute {
	boost::uint64_t operator()(boost::uint64_t i) const{
		return (i & 0xAAAAAAAAAAAAAAAALL) >> 1 | (i & 0x5555555555555555LL) << 1;
	}
};

template <typename T=std::less<uint64_t> >
struct bit_pertume_compare: std::binary_function<boost::uint64_t, boost::uint64_t, bool> {
	bit_permute bp;
	T c;
	typedef boost::uint64_t first_argument_type;
	typedef boost::uint64_t second_argument_type;

	bool operator()(boost::uint64_t a, boost::uint64_t b) const {
		return c(bp(a), bp(b));
	}
};

struct memory_monitor {
	tpie::size_type base;
	tpie::size_type used;
	inline void begin() {
		used = base = tpie::get_memory_manager().used();
	}
	inline void sample() {
		used = std::max(used, tpie::get_memory_manager().used());
	}
	inline void clear() {
		used = tpie::get_memory_manager().used();
	}
	inline void empty() {
		used = base;
	}
	inline tpie::size_type usage() {
		return used-base;
	}
};

class memory_test {
public:
	virtual void free() = 0;
	virtual void alloc() = 0;
	virtual void use() {}
	virtual tpie::size_type claimed_size() = 0;
	bool operator()() {
		bool res=true;
		tpie::get_memory_manager().set_limit(128*1024*1024);
		tpie::size_type g = claimed_size();
		memory_monitor mm;
		mm.begin();
		alloc();
		mm.sample();
		use();
		mm.sample();
		if (mm.usage() > g) {
			std::cerr << "Claimed to use " << g << " but used " << mm.usage() << std::endl;
			res=false;
		}
		free();
		mm.empty();
		mm.sample();
		if (mm.usage() > 0) {
			std::cerr << "Leaked memory " << mm.usage() << std::endl;
			res=false;
		}
		return res;
	}
};

class tpie_initer {
public:
	tpie_initer(size_t memory_limit=50) {
		tpie::tpie_init();
		tpie::get_memory_manager().set_limit(memory_limit*1024*1024);
	}
	
	~tpie_initer() {
		tpie::tpie_finish();
	}
};

#endif //__COMMON_H__
