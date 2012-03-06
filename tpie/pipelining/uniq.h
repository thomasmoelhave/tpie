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

#ifndef __TPIE_PIPELINING_UNIQ_H__
#define __TPIE_PIPELINING_UNIQ_H__

#include <tpie/pipelining/core.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/file_stream.h>

namespace tpie {

namespace pipelining {

template <typename dest_t>
struct count_consecutive_t : public pipe_segment {
	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	~count_consecutive_t() {}

	typedef uint64_t count_t;
	typedef typename dest_t::item_type::first_type item_type;

	inline count_consecutive_t(const dest_t & dest)
		: dest(dest)
		, current_count(0)
	{
	}

	inline void begin() {
		dest.begin();
	}

	inline void end() {
		flush();
		dest.end();
	}

	inline void push(const item_type & item) {
		if (current_count && item == item_buffer) {
			++current_count;
		} else {
			flush();
			item_buffer = item;
			current_count = 1;
		}
	}

	const pipe_segment * get_next() const {
		return &dest;
	}
private:
	inline void flush() {
		if (!current_count) return;
		dest.push(std::make_pair(item_buffer, current_count));
		current_count = 0;
	}
	dest_t dest;
	item_type item_buffer;
	count_t current_count;
};

namespace bits {

struct any_type {
	template <typename T>
	inline any_type(const T &) {}
	template <typename T>
	inline any_type & operator=(const T &) {return *this;}
};

}

template <typename dest_t>
struct extract_first_t : public pipe_segment {
	///////////////////////////////////////////////////////////////////////////
	/// \brief Virtual dtor.
	///////////////////////////////////////////////////////////////////////////
	~extract_first_t() {}

	typedef std::pair<typename dest_t::item_type, bits::any_type> item_type;

	inline extract_first_t(const dest_t & dest) : dest(dest) {
	}

	inline void begin() {
		dest.begin();
	}

	inline void end() {
		dest.end();
	}

	inline void push(const item_type & item) {
		dest.push(item.first);
	}

	const pipe_segment * get_next() const {
		return &dest;
	}
private:
	dest_t dest;
};

inline pipe_middle<bits::pair_factory<factory_0<count_consecutive_t>, factory_0<extract_first_t> > >
pipeuniq() {
	return bits::pair_factory<factory_0<count_consecutive_t>, factory_0<extract_first_t> >
		(factory_0<count_consecutive_t>(), factory_0<extract_first_t>());
}

}

}

#endif
