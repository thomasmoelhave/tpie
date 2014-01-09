// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, 2013, The TPIE development team
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

#ifndef __TPIE_PIPELINING_MERGER_H__
#define __TPIE_PIPELINING_MERGER_H__

#include <tpie/internal_priority_queue.h>
#include <tpie/file_stream.h>
#include <tpie/tpie_assert.h>
#include <tpie/pipelining/run_file_container.h>

namespace tpie {

template <typename T, typename pred_t>
class merger {
public:
	inline merger(pred_t pred)
		: m_priority_queue(0, predwrap(pred))
	{
	}

	inline bool can_pull() {
		return !m_priority_queue.empty();
	}

	inline T pull() {
		tp_assert(can_pull(), "pull() while !can_pull()");
		T el = m_priority_queue.top().first;
		memory_size_type i = m_priority_queue.top().second;

		if(m_containers[i].can_pull()) {
			m_priority_queue.pop_and_push(std::make_pair(m_containers[i].pull(), i));
		} 
		else {
			m_priority_queue.pop();
		}
		
		if(!can_pull()) {
			reset();
		}

		return el;
	}

	inline void reset() {
		m_priority_queue.resize(0);
	}

	// Initialize merger with given sorted input runs. Each file stream is
	// assumed to have a stream offset pointing to the first item in the run.
	// Precondition: !can_pull()
	void reset(bits::run_file_container<T> * containers, memory_size_type fanout = 0) {
		tp_assert(m_priority_queue.empty(), "Reset before we are done");

		m_containers = containers;
		m_fanout = fanout;

		m_priority_queue.resize(fanout);
		for(memory_size_type i = 0; i < fanout; ++i) {
			if(m_containers[i].can_pull()) {
				m_priority_queue.unsafe_push(std::make_pair(m_containers[i].pull(), i));
			}
		}

		m_priority_queue.make_safe();
	}

	inline static memory_size_type memory_usage(memory_size_type fanout) {
		return sizeof(merger)
			- sizeof(internal_priority_queue<std::pair<T, memory_size_type>, predwrap>) // m_priority_queue
			+ static_cast<memory_size_type>(internal_priority_queue<std::pair<T, memory_size_type>, predwrap>::memory_usage(fanout)) // m_priority_queue
			;
	}

	class predwrap {
	public:
		typedef std::pair<T, memory_size_type> item_type;
		typedef item_type first_argument_type;
		typedef item_type second_argument_type;
		typedef bool result_type;

		predwrap(pred_t pred)
			: pred(pred)
		{
		}

		inline bool operator()(const item_type & lhs, const item_type & rhs) {
			return pred(lhs.first, rhs.first);
		}

	private:
		pred_t pred;
	};

private:
	internal_priority_queue<std::pair<T, memory_size_type>, predwrap> m_priority_queue;
	bits::run_file_container<T> * m_containers;
	memory_size_type m_fanout;
};

} // namespace tpie

#endif // __TPIE_PIPELINING_MERGER_H__
