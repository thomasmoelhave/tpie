// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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

#ifndef __TPIE_PIPELINING_BUFFER_H__
#define __TPIE_PIPELINING_BUFFER_H__

#include <tpie/pipelining/pipe_segment.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/file_stream.h>

namespace tpie {
namespace pipelining {


template <typename T>
struct buffer_input_t: public pipe_segment {
public:
	typedef T item_type;
	buffer_input_t(file_stream<T> & queue, const segment_token & token):
		pipe_segment(token), queue(queue) {
		set_name("Storing items", PRIORITY_SIGNIFICANT);
		set_minimum_memory(queue.memory_usage());
	}

	virtual void begin() /*override*/ {
		pipe_segment::begin();
		queue.open();
	}
	void push(const T & item) {queue.write(item);}
private:
	file_stream<T> & queue;

};

template <typename T>
struct buffer_pull_output_t: public pipe_segment {
	typedef T item_type;

	buffer_pull_output_t(file_stream<T> & queue, const segment_token & input_token)
		: queue(queue) {
		add_dependency(input_token);
		set_name("Fetching items", PRIORITY_SIGNIFICANT);
		set_minimum_memory(queue.memory_usage());
	}


	file_stream<T> & queue;
	virtual void begin() /*override*/ {
		pipe_segment::begin();
		queue.seek(0);
		forward("items", queue.size());
	}
	bool can_pull() const {return queue.can_read();}
	T pull() {return queue.read();}
	virtual void end() /*override*/ {
		pipe_segment::end();
		queue.close();
	}
};


template <typename T>
struct passive_buffer {
	typedef T item_type;
	typedef buffer_input_t<T> input_t;
	typedef buffer_pull_output_t<T> output_t;
private:
	typedef termfactory_2<input_t,  file_stream<T> &, const segment_token &> inputfact_t;
	typedef termfactory_2<output_t, file_stream<T> &, const segment_token &> outputfact_t;
	typedef pipe_end      <inputfact_t>  inputpipe_t;
	typedef pullpipe_begin<outputfact_t> outputpipe_t;

public:
	passive_buffer() {}

	inline input_t raw_input() {
		return input_t(queue, input_token);
	}

	inline output_t raw_output() {
		return output_t(queue, input_token);
	}

	inline inputpipe_t input() {
		return inputfact_t(queue, input_token);
	}

	inline outputpipe_t output() {
		return outputfact_t(queue, input_token);
	}

private:
	segment_token input_token;
	file_stream<T> queue;

	passive_buffer(const passive_buffer &);
	passive_buffer & operator=(const passive_buffer &);
};

}
}

#endif // __TPIE_PIPELINING_BUFFER_H__
