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
#include <tpie/types.h>
#include <tpie/streaming/memory.h>
#include <tpie/streaming/util.h>

namespace tpie {
namespace streaming {

template <typename item_t, 
		  memory_size_type buff_size,
		  typename begin_data_t=empty_type,
		  typename end_data_t=empty_type>
class virtual_source_real: public memory_single {
public:
	typedef begin_data_t begin_data_type;
	typedef end_data_t end_data_type;
	typedef item_t item_type;
	
	virtual void begin(stream_size_type items=max_items, begin_data_type * data=0) = 0;
	virtual void push(const item_t * items, memory_size_type count) = 0;
	virtual void end(end_data_type * data = 0) = 0;
};

template <typename item_t, 
		  typename begin_data_t,
		  typename end_data_t>
class virtual_source_real<item_t, 1, begin_data_t, end_data_t>: public memory_single {
public:
	typedef begin_data_t begin_data_type;
	typedef end_data_t end_data_type;
	typedef item_t item_type;

	virtual void begin(stream_size_type items=max_items, begin_data_type *data=0) = 0;
	virtual void push(const item_t & item) = 0;
	virtual void end(end_data_type * data=0) = 0;
};


template <typename item_t, 
		  memory_size_type buff_size,
		  typename begin_data_t=empty_type,
		  typename end_data_t=empty_type>
class virtual_sink_real_impl
	: public common_single< virtual_sink_real_impl<item_t , buff_size, begin_data_t, end_data_t>, virtual_source_real<item_t, buff_size, begin_data_t, end_data_t> > {
private:
	typedef virtual_source_real<item_t, buff_size, begin_data_t, end_data_t> dest_t;
	typedef common_single< virtual_sink_real_impl<item_t , buff_size, begin_data_t, end_data_t>, dest_t> parent_t;
	using parent_t::dest;
private:
 	item_t m_buffer[buff_size];
 	memory_size_type m_bufferUsed;
	
 	inline void flush() {
 		dest().push(m_buffer, m_bufferUsed);
 		m_bufferUsed = 0;
 	}
public:
 	inline virtual_sink_real_impl(dest_t * dest) throw ()
		:parent_t(*dest, 0.0), m_bufferUsed(0) {}
	
 	inline void begin(stream_size_type items=max_items, begin_data_t * data=0) {
 		m_bufferUsed = 0;
		dest().begin(items, data);
 	}
	
 	void push(const item_t & item) {
 		m_buffer[m_bufferUsed++] = item;
 		if (m_bufferUsed == buff_size) flush();
 	}

 	void end(end_data_t * data=0) {
 		if (m_bufferUsed != 0) flush();
		dest().end(data);
 	}
};

template <typename item_t, 
		  typename begin_data_t,
		  typename end_data_t>
class virtual_sink_real_impl<item_t, 1, begin_data_t, end_data_t>
	: public common_single< virtual_sink_real_impl<item_t, 1, begin_data_t, end_data_t>, virtual_source_real<item_t, 1, begin_data_t, end_data_t> > {
private:
	typedef virtual_source_real<item_t, 1, begin_data_t, end_data_t> dest_t;
	typedef common_single< virtual_sink_real_impl<item_t, 1, begin_data_t, end_data_t>, dest_t > parent_t;
	using parent_t::dest;
public:
	virtual_sink_real_impl(dest_t * dest): parent_t(*dest, 0.0) {}
	inline void begin(stream_size_type items=max_items, begin_data_t * data=0) {
		dest().begin(items, data);
	}
	inline void push(const item_t & item) {
		dest().push(item);
	}
	inline void end(end_data_t * data=0) {
		dest().end(data);
	}
};

template <typename dest_t, unsigned buff_size>
class virtual_source_impl_real
	: public virtual_source_real<typename dest_t::item_type, buff_size, typename dest_t::begin_data_type, typename dest_t::end_data_type> {
private:
	typedef virtual_source_real<typename dest_t::item_type, buff_size, typename dest_t::begin_data_type, typename dest_t::end_data_type> parent_t;
 	dest_t & m_dest;
public:
	typedef typename parent_t::item_type item_type;
	typedef typename parent_t::begin_data_type begin_data_type;
	typedef typename parent_t::end_data_type end_data_type;

 	virtual_source_impl_real(dest_t & dest): m_dest(dest) {}

	virtual void begin(stream_size_type items=max_items, begin_data_type * data=0) {
		m_dest.begin(items, data);
	}
	
	virtual void end(end_data_type * data=0) {
		m_dest.end(data);
	}
	
 	virtual void push(const item_type * items, memory_size_type count) {
 		for(memory_size_type i=0; i < count; ++i)
 			m_dest.push(items[i]);
 	}
	
	virtual memory_size_type base_memory() {
		return sizeof(*this);
	}

	void memory_next(std::vector<memory_base *> &ds) {
		ds.push_back(&m_dest);
	}
};

template <typename dest_t>
class virtual_source_impl_real<dest_t, 1>
	: public virtual_source_real<typename dest_t::item_type, 1, typename dest_t::begin_data_type, typename dest_t::end_data_type> {
private:
	typedef virtual_source_real<typename dest_t::item_type, 1, typename dest_t::begin_data_type, typename dest_t::end_data_type>  parent_t;
 	dest_t & m_dest;
public:
	typedef typename parent_t::item_type item_type;
	typedef typename parent_t::begin_data_type begin_data_type;
	typedef typename parent_t::end_data_type end_data_type;

 	virtual_source_impl_real(dest_t & dest): m_dest(dest) {};
	virtual void begin(memory_size_type items=max_items, begin_data_type * data=0) {
		m_dest.begin(items, data);
	}
	
	virtual void end(end_data_type * data=0) {
		m_dest.end(data);
	}

 	virtual void push(const item_type & item) {
		m_dest.push(item);
	}

	virtual memory_size_type base_memory() {
		return sizeof(*this);
	}

	void memory_next(std::vector<memory_base *> &ds) {
		ds.push_back(&m_dest);
	}
};

}
}




