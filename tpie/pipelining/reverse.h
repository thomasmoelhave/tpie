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

#ifndef __TPIE_PIPELINING_REVERSE_H__
#define __TPIE_PIPELINING_REVERSE_H__

#include <tpie/pipelining/core.h>
#include <tpie/pipelining/factory_helpers.h>

namespace tpie {

namespace pipelining {

template <typename T>
struct reverser {
	typedef std::vector<T> buf_t;

	struct sink_t : public pipe_segment {
		///////////////////////////////////////////////////////////////////////
		/// \brief Virtual dtor.
		///////////////////////////////////////////////////////////////////////
		~sink_t() {}

		typedef T item_type;

		inline sink_t(buf_t & buffer, const segment_token & token)
			: pipe_segment(token)
			, buffer(buffer)
		{
			it = buffer.begin();
		}

		inline void begin() {
		}

		inline void end() {
		}

		inline void push(const T & item) {
			*it++ = item;
		}

	private:
		buf_t & buffer;
		typename buf_t::iterator it;
	};

	template <typename dest_t>
	struct source_t : public pipe_segment {
		///////////////////////////////////////////////////////////////////////
		/// \brief Virtual dtor.
		///////////////////////////////////////////////////////////////////////
		~source_t() {}

		typedef T item_type;

		inline source_t(const dest_t & dest, const buf_t & buffer, const segment_token & sink)
			: dest(dest)
		   	, buffer(buffer)
			, it(buffer.rbegin())
		{
			add_push_destination(dest);
			add_dependency(sink);
		}

		inline void go(progress_indicator_base & pi) {
			pi.init(buffer.size());
			dest.begin();
			while (it != buffer.rend()) {
				dest.push(*it++);
				pi.step();
			}
			dest.end();
			pi.done();
		}

	private:
		dest_t dest;
		const buf_t & buffer;
		typename buf_t::const_reverse_iterator it;

		source_t & operator=(const source_t & other);
	};

	inline reverser(size_t buffer_size)
		: buffer(buffer_size)
	{
	}

	inline pipe_end<termfactory_2<sink_t, buf_t &, const segment_token &> >
	sink() {
		return termfactory_2<sink_t, buf_t &, const segment_token &>(buffer, sink_token);
	}

	inline pipe_begin<factory_2<source_t, const buf_t &, const segment_token &> >
	source() {
		return factory_2<source_t, const buf_t &, const segment_token &>(buffer, sink_token);
	}

private:
	buf_t buffer;
	segment_token sink_token;
};

}

}

#endif
