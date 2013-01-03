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

///////////////////////////////////////////////////////////////////////////////
/// \file pipelining/buffer.h  Plain old file_stream buffer.
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_PIPELINING_BUFFER_H__
#define __TPIE_PIPELINING_BUFFER_H__

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/file_stream.h>

namespace tpie {

namespace pipelining {

namespace bits {

///////////////////////////////////////////////////////////////////////////////
/// \brief Input node for buffer.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class buffer_input_t: public node {
public:
	typedef T item_type;
	buffer_input_t(file_stream<T> & queue, const node_token & token)
		: node(token)
		, queue(queue)
	{
		set_name("Storing items", PRIORITY_SIGNIFICANT);
		set_minimum_memory(queue.memory_usage());
	}

	virtual void begin() /*override*/ {
		node::begin();
		queue.open();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc node::push
	///////////////////////////////////////////////////////////////////////////
	void push(const item_type & item) {
		queue.write(item);
	}

private:
	file_stream<T> & queue;
};

template <typename T>
class buffer_pull_output_t: public node {
	file_stream<T> & queue;

public:
	typedef T item_type;

	buffer_pull_output_t(file_stream<T> & queue, const node_token & input_token)
		: queue(queue)
	{
		add_dependency(input_token);
		set_name("Fetching items", PRIORITY_SIGNIFICANT);
		set_minimum_memory(queue.memory_usage());
	}

	virtual void begin() /*override*/ {
		node::begin();
		queue.seek(0);
		forward("items", queue.size());
	}

	bool can_pull() const {
		return queue.can_read();
	}

	T pull() {
		return queue.read();
	}

	virtual void end() /*override*/ {
		queue.close();
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Input node for delayed buffer.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class delayed_buffer_input_t: public node {
public:
	typedef T item_type;

	delayed_buffer_input_t(const node_token & token)
		: node(token)
	{
		set_name("Storing items", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(tpie::file_stream<item_type>::memory_usage());
	}

	virtual void begin() /*override*/ {
		node::begin();
		m_queue = tpie::tpie_new<tpie::file_stream<item_type> >();
		m_queue->open();
		forward("queue", m_queue);
	}

	void push(const T & item) {
		m_queue->write(item);
	}

private:
	tpie::file_stream<T> * m_queue;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Output node for delayed buffer.
///////////////////////////////////////////////////////////////////////////////
template <typename dest_t>
class delayed_buffer_output_t: public node {
public:
	typedef typename dest_t::item_type item_type;

	delayed_buffer_output_t(const dest_t &dest, const node_token & input_token)
		: dest(dest)
	{
		add_dependency(input_token);
		add_push_destination(dest);
		set_minimum_memory(tpie::file_stream<item_type>::memory_usage());
		set_name("Fetching items", PRIORITY_INSIGNIFICANT);
	}

	virtual void begin() /*override*/ {
		m_queue = fetch<tpie::file_stream<item_type> *>("queue");
		forward("items", m_queue->size());
		set_steps(m_queue->size());
	}

	virtual void go() /*override*/ {
		m_queue->seek(0);
		while (m_queue->can_read()) {
			dest.push(m_queue->read());
			step();
		}
	}

	virtual void end() /*override*/ {
		tpie::tpie_delete(m_queue);
	}

private:
	dest_t dest;
	file_stream<item_type> * m_queue;
};



} // namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Plain old file_stream buffer. Does nothing to the item stream, but
/// it inserts a phase boundary.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class passive_buffer {
public:
	typedef T item_type;
	typedef bits::buffer_input_t<T> input_t;
	typedef bits::buffer_pull_output_t<T> output_t;
private:
	typedef termfactory_2<input_t,  file_stream<T> &, const node_token &> inputfact_t;
	typedef termfactory_2<output_t, file_stream<T> &, const node_token &> outputfact_t;
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
	node_token input_token;
	file_stream<T> queue;

	passive_buffer(const passive_buffer &);
	passive_buffer & operator=(const passive_buffer &);
};

template <typename dest_t>
class delayed_buffer_t: public node {
public:
	typedef typename dest_t::item_type item_type;
	typedef bits::delayed_buffer_input_t<item_type> input_t;
	typedef bits::delayed_buffer_output_t<dest_t> output_t;

	delayed_buffer_t(const dest_t & dest)
		: input_token()
		, input(input_token)
		, output(dest, input_token)
	{
		add_push_destination(input);
		set_name("Delayed buffer", PRIORITY_INSIGNIFICANT);
	}

	delayed_buffer_t(const delayed_buffer_t &o)
		: node(o)
		, input_token(o.input_token)
		, input(o.input)
		, output(o.output)
	{
	}

	virtual void push(item_type item) {
		input.push(item);
	}

	node_token input_token;

	input_t input;
	output_t output;
};

inline pipe_middle<factory_0<delayed_buffer_t> > delayed_buffer() {
	return factory_0<delayed_buffer_t>();
}

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_BUFFER_H__
