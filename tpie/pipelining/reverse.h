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
#include <tpie/stack.h>

namespace tpie {

namespace pipelining {

template <typename T>
struct passive_reverser {
	typedef std::vector<T> buf_t;

	struct sink_t : public pipe_segment {
		typedef T item_type;

		inline sink_t(buf_t & buffer, const segment_token & token)
			: pipe_segment(token)
			, buffer(buffer)
		{
			it = buffer.begin();
			set_name("Input items to reverse", PRIORITY_INSIGNIFICANT);
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
		typedef T item_type;

		inline source_t(const dest_t & dest, const buf_t & buffer, const segment_token & sink)
			: dest(dest)
		   	, buffer(buffer)
			, it(buffer.rbegin())
		{
			add_push_destination(dest);
			add_dependency(sink);
			set_name("Output reversed items", PRIORITY_INSIGNIFICANT);
		}

		virtual void begin() /*override*/ {
			forward("items", static_cast<stream_size_type>(buffer.size()));
		}

		inline void go(progress_indicator_base & pi) {
			pi.init(buffer.size());
			while (it != buffer.rend()) {
				dest.push(*it++);
				pi.step();
			}
			pi.done();
		}

	private:
		dest_t dest;
		const buf_t & buffer;
		typename buf_t::const_reverse_iterator it;

		source_t & operator=(const source_t & other);
	};

	inline passive_reverser(size_t buffer_size)
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

namespace bits {

template <typename T>
struct reverser_input_t: public pipe_segment {
	typedef T item_type;

	inline reverser_input_t(const segment_token & token, stack<T> & the_stack):
		pipe_segment(token), the_stack(&the_stack) {
		set_name("Store items", PRIORITY_SIGNIFICANT);
	}

	void push(const T & t) {
		the_stack->push(t);
	}

	stack<T> * the_stack;
};

template <typename dest_t>
struct reverser_output_t: public  pipe_segment {
	typedef typename dest_t::item_type item_type;
	
	reverser_output_t(const dest_t & dest, const segment_token & input_token, stack<item_type> & the_stack)
		: dest(dest), the_stack(&the_stack) {
		add_dependency(input_token);
		add_push_destination(dest);
	}

	virtual void begin() /*override*/ {
		pipe_segment::begin();
		forward("items", the_stack->size());
	}

	virtual void go(progress_indicator_base & pi) /*override*/ {
		pi.init(the_stack->size());
		while (!the_stack->empty()) {
			dest.push(the_stack->pop());
		}
		pi.done();
	}

	dest_t dest;
	stack<item_type> * the_stack;
};


template <typename dest_t>
struct reverser_t: public pipe_segment {
	typedef typename dest_t::item_type item_type;
	
	typedef reverser_output_t<dest_t> output_t;
	typedef reverser_input_t<item_type> input_t;
	
	inline reverser_t(const dest_t & dest):
		input_token(), input(input_token, the_stack), output(dest, input_token, the_stack) {
		add_push_destination(input);
	}

	inline reverser_t(const reverser_t & o):
		pipe_segment(o), input_token(o.input_token), input(o.input), output(o.output) {
		input.the_stack = &the_stack;
		output.the_stack = &the_stack;
	}

	void push(const item_type & i) {input.push(i);}
private:
	segment_token input_token;
	stack<item_type> the_stack;

	input_t input;
	output_t output;
};
}

inline pipe_middle<factory_0<bits::reverser_t> > reverser() {return factory_0<bits::reverser_t>();}

}
}
#endif
