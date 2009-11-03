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

namespace tpie {
namespace streaming {

template <class item_t>
class null_sink: public memory_single {
public:
	typedef item_t item_type;
	inline void begin(TPIE_OS_OFFSET size=0) {}
	inline void push(const item_type & item) {}
	inline void end() {}
	virtual TPIE_OS_SIZE_T memoryBase() {return sizeof(null_sink);}

	null_sink() {
		setMemoryPriority(0);
	}
};

template <typename dest_t> 
class common_single: public memory_single {
private:
	dest_t & d;
protected:
	dest_t & dest() {return d;}
	
    common_single(dest_t & de, double prio): d(de) {
		setMemoryPriority(prio);
	}
	
	void memoryNext(std::vector<memory_base *> &ds) {
		ds.push_back(&d);
	}
		

	TPIE_OS_SIZE_T minimumMemory() {
		return memoryBase();
	};
};


template <typename dest_t>
class common_split: public memory_split {
private:
	dest_t & d;
protected:
	dest_t & dest() {return d;}

    common_split(dest_t & de): d(de) {}
	
	void memoryNext(std::vector<memory_base *> &ds) {
		ds.push_back(&d);
	}
};

template <typename first_t> 
class common_wrapper: public memory_wrapper {
private:
	first_t * f;
public:
	typedef typename first_t::item_type item_type;
	
	common_wrapper(first_t * fi): f(fi) {}

	inline void begin(TPIE_OS_SIZE_T count=0) {
		f->begin(count);
	}

	inline void end() {
		f->end();
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
