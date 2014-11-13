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

#ifndef __TPIE_PIPELINING_INTERNAL_BUFFER_H__
#define __TPIE_PIPELINING_INTERNAL_BUFFER_H__

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/internal_queue.h>
#include <tpie/tpie_assert.h>

namespace tpie {

namespace pipelining {

namespace bits {

template <typename T>
class internal_buffer_pull_output_t: public node {
	internal_queue<T> * m_queue;
public:
	typedef T item_type;

	internal_buffer_pull_output_t(const node_token & input_token, size_t size) {
		add_pull_source(input_token);
		set_name("Fetching items", PRIORITY_SIGNIFICANT);
		set_minimum_memory(internal_queue<T>::memory_usage(size));
	}

	virtual void propagate() override {
		m_queue = fetch<internal_queue<T> *>("queue");
	}

	bool can_pull() const {
		return !m_queue->empty();
	}

	T pull() {
		T item=m_queue->front();
		m_queue->pop();
		return item;
	}

	virtual void end() override {
		tpie_delete(m_queue);
		m_queue=NULL;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Input node for buffer.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class internal_buffer_input_t: public node {
public:
	typedef T item_type;

	internal_buffer_input_t(const node_token & token, size_t size)
		: node(token), size(size) {
		set_name("Storing items", PRIORITY_INSIGNIFICANT);
		set_minimum_memory(internal_queue<T>::memory_usage(size));
		set_plot_options(PLOT_SIMPLIFIED_HIDE);
	}

	virtual void propagate() override {
		m_queue = tpie::tpie_new<internal_queue<item_type> >(size);
		forward("queue", m_queue);
	}

	void push(const T & item) {
		tp_assert(!m_queue->full(), "Tried to push into full queue");
		m_queue->push(item);
	}

private:
	internal_queue<item_type> * m_queue;
	size_t size;
};

} //namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Plain old file_stream buffer. Does nothing to the item stream, but
/// it inserts a phase boundary.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class internal_passive_buffer {
public:
	typedef T item_type;
	typedef bits::internal_buffer_input_t<T> input_t;
	typedef bits::internal_buffer_pull_output_t<T> output_t;
private:
	typedef termfactory_2<input_t,  const node_token &, size_t> inputfact_t;
	typedef termfactory_2<output_t, const node_token &, size_t> outputfact_t;
	typedef pipe_end      <inputfact_t>  inputpipe_t;
	typedef pullpipe_begin<outputfact_t> outputpipe_t;

public:
	internal_passive_buffer(size_t size) {}

	inline input_t raw_input() {
		return input_t(input_token, size);
	}

	inline output_t raw_output() {
		return output_t(input_token, size);
	}

	inline inputpipe_t input() {
		return inputfact_t(input_token, size);
	}

	inline outputpipe_t output() {
		return outputfact_t(input_token, size);
	}
	
private:
	node_token input_token;
	size_t size;
	
	internal_passive_buffer(const internal_passive_buffer &);
	internal_passive_buffer & operator=(const internal_passive_buffer &);
};

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_INTERNAL_BUFFER_H__
