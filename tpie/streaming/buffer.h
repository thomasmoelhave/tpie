// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team
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

#ifndef _TPIE_STREAMING_BUFFER_H
#define _TPIE_STREAMING_BUFFER_H
#include <tpie/streaming/stream.h>
#include <tpie/streaming/memory.h>
#include <utility>
#include <tpie/streaming/concepts.h>
#include <tpie/mm_manager.h>

namespace tpie {
namespace streaming {

template <typename item_t, 
		  typename begin_data_t=empty_type>
class buffer_base: public memory_split {
public:
	typedef item_t item_type;
	typedef begin_data_t begin_data_type;
protected:
	item_t * m_buff;
	memory_size_type m_buffSize;
	memory_size_type m_buffIndex;
	file_stream<item_type> * m_stream;
	double m_blockFactor;
	begin_data_t * m_beginData;
public:
	buffer_base(double blockFactor): m_buff(0), m_stream(0), m_blockFactor(blockFactor), m_beginData(0) {}

	memory_size_type minimum_memory_in() {
		return base_memory() 
			+ 2*MM_manager.space_overhead()
			+ file_stream<item_type>::memory_usage(1, m_blockFactor);
	}
	
	inline void begin(stream_size_type items=max_items, begin_data_t * data=0) {
		memory_size_type bs = std::min(memory_in() - minimum_memory_in(), 
									   memory_out() - minimum_memory_out());
		m_buffIndex = 0;
		m_buffSize = std::min(items, stream_size_type(bs/sizeof(item_t)));
		if (m_buffSize > 0)
			m_buff = new item_type[m_buffSize];
		m_stream = 0;
		m_beginData = data;
	}
	
	inline void push(const item_type & item) {
		if (m_stream == 0) {
			if (m_buffIndex < m_buffSize) {
				m_buff[m_buffIndex++] = item;
				return;
			}
			m_stream = new file_stream<item_type>(m_blockFactor);
			m_stream->open();
		}
		m_stream->write(item);
	}

	inline stream_size_type calculate_size() {
		return stream_size_type(m_buffIndex) + (m_stream?m_stream->size():stream_size_type(0));
	}
};

 		
template <typename T, 
		  bool backwards=false,
		  typename begin_data_t=empty_type, 
		  typename end_data_t=empty_type>
class pull_buffer: public buffer_base<T, begin_data_t> {
public:
	typedef T item_type;
	typedef T pull_type;
	typedef end_data_t end_data_type;
private:
	typedef buffer_base<T, begin_data_t> parent_t;
	using parent_t::m_buff;
	using parent_t::m_buffIndex;
	using parent_t::m_stream;
	using parent_t::base_memory;
	using parent_t::m_blockFactor;
	using parent_t::calculate_size;
	typedef pull_stream_source<item_type, backwards> source_t;
	pull_stream_source<item_type> * m_source;
	
	memory_size_type m_index;
	end_data_type * m_endData;
public:
	memory_size_type base_memory() {
		return sizeof(*this);
	}
	
	pull_buffer(double blockFactor=1.0): parent_t(blockFactor), m_source(0), m_index(0) {};

	void end(end_data_type * data=0) {
		m_endData=data;
	}

	memory_size_type minimum_memory_out() {
		return base_memory()
			+ 3*MM_manager.space_overhead()
			+ file_stream<item_type>::memory_usage(m_blockFactor, 1)
			+ source_t::memory();
	}
	
	inline void pull_begin(stream_size_type * size=0) {
		if (size) 
			*size = calculate_size();
		if (m_stream) {
			m_source = new source_t(*m_stream);
			m_source->pull_begin();
		}
		if (backwards)
			m_index=m_buffIndex-1;
		else
			m_index=0;
	}
	
	const item_type & pull() {
		if (backwards)
			return m_source->can_pull() ? m_source->pull() : m_buff[m_index--];
		else
			return (m_index < m_buffIndex) ? m_buff[m_index++] : m_source->pull();
	}

	bool can_pull() {
		if (backwards)
			return m_index != memory_size_type(-1) || (m_source	&& m_source->can_pull());
		else {
			return m_index != m_buffIndex || (m_source && m_source->can_pull());
		}
	}

	void pull_end() {
		if (m_buff)
			delete[] m_buff;
		m_buff=0;
		if (m_source) {
			m_source->pull_end();
			delete m_source;
		}
		m_source = 0;
		if (m_stream)
			delete m_stream;
		m_stream = 0;
	}
};

template <class dest_t, bool backwards=false> 
class buffer: public buffer_base<typename dest_t::item_type, typename dest_t::begin_data_type> {
public:
	typedef typename dest_t::item_type item_type;
	typedef typename dest_t::begin_data_type begin_data_type;
	typedef typename dest_t::end_data_type end_data_type;
	
private:
	typedef buffer_base<item_type, begin_data_type> parent_t;
	typedef pull_stream_source<item_type, backwards> source_t;
	BOOST_CONCEPT_ASSERT((tpie::streaming::concepts::pushable<dest_t>));

	using parent_t::m_buff;
	using parent_t::m_buffIndex;
	using parent_t::m_stream;
	using parent_t::base_memory;
	using parent_t::m_blockFactor;
	using parent_t::m_beginData;
	using parent_t::calculate_size;
	dest_t & m_dest;
public:
	memory_size_type base_memory() {
		return sizeof(*this);
	}

	buffer(dest_t & dest, double blockFactor=1.0): parent_t(blockFactor), m_dest(dest) {};

	memory_size_type minimum_memory_out() {
		//TODO substract allocation overhead here
		return base_memory()
			+ file_stream<item_type>::memory_usage(1, m_blockFactor) 
			+ 2*MM_manager.space_overhead();
	}

	void end(end_data_type * endData=0) {
		m_dest.begin(calculate_size(), m_beginData);
		if (!backwards) 
			for (memory_size_type i=0; i < m_buffIndex; ++i)
				m_dest.push(m_buff[i]);

		if (m_stream) {
			{
				source_t source(*m_stream);
				source.pull_begin();
				while (source.can_pull())
					m_dest.push(source.pull());
				source.pull_end();
			}
			delete m_stream;
			m_stream = 0;
		}
		
		if (backwards) 
			for (memory_size_type i=m_buffIndex-1; i != memory_size_type(-1); --i)
				m_dest.push(m_buff[i]);
		
		if (m_buff) 
			delete[] m_buff;
		m_buff = 0;
		m_dest.end(endData);
	}
	
	virtual void memoryNext(std::vector<memory_base *> & next) {
		next.push_back(&m_dest);
	}
};

}
}

#endif // _TPIE_STREAMING_BUFFER_H
