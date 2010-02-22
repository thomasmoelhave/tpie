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
#ifndef _TPIE_STREAMING_UTIL_H
#define _TPIE_STREAMING_UTIL_H
#include <limits>

namespace tpie {
namespace streaming {

const stream_size_type max_items=std::numeric_limits<stream_size_type>::max();
class empty_type {};

template <class item_t, class begin_data_t=empty_type, class end_data_t=empty_type>
class null_sink: public memory_single {
public:
	typedef begin_data_t begin_data_type;
	typedef end_data_t end_data_type;
	typedef item_t item_type;
	inline void begin(stream_size_type items=max_items,
					  begin_data_type * data=0) {}
	inline void push(const item_type & item) {}
	inline void end(end_data_type * data=0) {}
	virtual memory_size_type base_memory() {
		return sizeof(*this);
	}
	null_sink() {
		set_memory_priority(0);
	}
};

template <typename super_t, 
		  typename dest_t, 
		  typename begin_data_t=typename dest_t::begin_data_type,
		  typename end_data_t=typename dest_t::end_data_type>
class common_single: public memory_single {
public:
	typedef begin_data_t begin_data_type;
	typedef end_data_t end_data_type;
private:
	dest_t & d;
protected:
	dest_t & dest() {return d;}
	
    common_single(dest_t & de, double prio): d(de) {
		set_memory_priority(prio);
	}
public:	
	void memory_next(std::vector<memory_base *> &ds) {
		ds.push_back(&d);
	}

	memory_size_type minimum_memory() {
		return base_memory();
	}
	
	virtual memory_size_type base_memory() {
		return sizeof(super_t);
	}

	void begin(stream_size_type items=max_items, begin_data_t * data=0) {
		d.begin(items, data);
	}
	
	void end(end_data_t & data=0) {
		d.end(data);
	}

};


template <typename dest_t>
class common_split: public memory_split {
private:
	dest_t & d;
protected:
	dest_t & dest() {return d;}

    common_split(dest_t & de): d(de) {}
	
	void memory_next(std::vector<memory_base *> &ds) {
		ds.push_back(&d);
	}
};

template <typename first_t>
class common_wrapper: public memory_wrapper {
private:
	first_t * f;
public:
	typedef typename first_t::item_type item_type;
	typedef typename first_t::begin_data_type begin_data_type;
	typedef typename first_t::end_data_type end_data_type;

	common_wrapper(first_t * fi): f(fi) {}

	inline void begin(stream_size_type items=max_items,
					  begin_data_type * data=0) {
		f->begin(items, data);
	}

	inline void end(end_data_type * data=0) {
		f->end(data);
	}
	
	inline void push(const typename first_t::item_type & item) {
		f->push(item);
	}

	virtual memory_base * first() {
		return f;
	}
};


}
}


#endif //_TPIE_STREAMING_UTIL_H
