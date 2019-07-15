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

#ifndef __TPIE_PIPELINING_MERGER_H__
#define __TPIE_PIPELINING_MERGER_H__

#include <tpie/internal_priority_queue.h>
#include <tpie/compressed/stream.h>
#include <tpie/file_stream.h>
#include <tpie/tpie_assert.h>
#include <tpie/pipelining/store.h>
namespace tpie {

template <typename specific_store_t, typename pred_t>
class merger {
private:
	typedef typename specific_store_t::store_type store_type;
	typedef typename specific_store_t::element_type element_type;

	typedef bits::store_pred<pred_t, specific_store_t> store_pred_t;
public:
	merger(pred_t pred, specific_store_t store,
				  memory_bucket_ref bucket = memory_bucket_ref())
		: pq(0, predwrap(store_pred_t(pred)), bucket)
		, in(bucket)
		, itemsRead(bucket)
		, m_store(store) {
	}

	bool can_pull() {
		return !pq.empty();
	}

	store_type pull() {
		tp_assert(can_pull(), "pull() while !can_pull()");
		store_type el = std::move(pq.top().first);
		size_t i = pq.top().second;
		if (in[i].can_read() && itemsRead[i] < runLength) {
			pq.pop_and_push(
				std::make_pair(m_store.element_to_store(in[i].read()), i));
			++itemsRead[i];
		} else {
			pq.pop();
		}
		if (!can_pull()) {
			reset();
		}
		return el;
	}

	void reset() {
		in.resize(0);
		pq.resize(0);
		itemsRead.resize(0);
	}

	// Initialize merger with given sorted input runs. Each file stream is
	// assumed to have a stream offset pointing to the first item in the run,
	// and runLength items are read from each stream (unless end of stream
	// occurs earlier).
	// Precondition: !can_pull()
	void reset(array<file_stream<element_type> > & inputs, stream_size_type runLength) {
		this->runLength = runLength;
		tp_assert(pq.empty(), "Reset before we are done");
		in.swap(inputs);
		pq.resize(in.size());
		for (size_t i = 0; i < in.size(); ++i) {
			pq.unsafe_push(
				std::make_pair(
					m_store.element_to_store(in[i].read()), i));
		}
		pq.make_safe();
		itemsRead.resize(in.size(), 1);
	}

	// Compute memory usage as a function of the fanout
	static constexpr linear_memory_usage memory_usage() noexcept {
		return
			linear_memory_usage(-sizeof(file_stream<element_type>) //in filestreams,
								+ file_stream<element_type>::memory_usage(), //in filestreams
								sizeof(merger) 
								- sizeof(internal_priority_queue<std::pair<store_type, size_t>, predwrap>) //pq
								- sizeof(array<file_stream<element_type> >) //in
								- sizeof(array<size_t>)) // itemsRead
			+ array<size_t>::memory_usage() //itemsRead
			+ internal_priority_queue<std::pair<store_type, size_t>, predwrap>::memory_usage() //pq
			+ array<file_stream<element_type> >::memory_usage(); //in
	}
	
	
	static constexpr memory_size_type memory_usage(memory_size_type fanout) noexcept {
		return memory_usage()(fanout);
	}

	class predwrap {
	public:
		typedef std::pair<store_type, size_t> item_type;
		typedef item_type first_argument_type;
		typedef item_type second_argument_type;
		typedef bool result_type;

		predwrap(store_pred_t pred)
			: pred(pred)
		{
		}

		inline bool operator()(const item_type & lhs, const item_type & rhs) {
			return pred(lhs.first, rhs.first);
		}

	private:
		store_pred_t pred;
	};

private:
	internal_priority_queue<std::pair<store_type, size_t>, predwrap> pq;
	array<file_stream<element_type> > in;
	array<stream_size_type> itemsRead;
	stream_size_type runLength;
	specific_store_t m_store;
};

} // namespace tpie

#endif // __TPIE_PIPELINING_MERGER_H__
