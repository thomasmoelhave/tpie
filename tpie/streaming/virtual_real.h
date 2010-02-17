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
namespace tpie {
namespace streaming {

template <typename item_t, unsigned buff_size>
class virtual_source_real {
public:
	virtual void begin(const size_t count=0) = 0;
	virtual void push(const item_t * items, size_t count) = 0;
	virtual void end() = 0;
};

template <typename item_t>
class virtual_source_real<item_t, 1> {
public:
	virtual void begin(size_t count=0) = 0;
	virtual void push(const item_t & item) = 0;
	virtual void end() = 0;
};


template <typename item_t, unsigned buff_size>
class virtual_sink_real_impl {
private:
	typedef virtual_source_real<item_t, buff_size> dest_t;
	dest_t * dest;
 	item_t buffer[buff_size];
 	size_t bufferUsed;

 	inline void flush() {
 		dest->push(buffer, bufferUsed);
 		bufferUsed = 0;
 	}
public:
 	virtual_sink_real_impl(dest_t * d): dest(d), bufferUsed(0) {};
	
 	inline void begin(size_t size=0) {
 		bufferUsed = 0;
		dest->begin(size);
 	}
	
 	void push(const item_t & item) {
 		buffer[bufferUsed++] = item;
 		if (bufferUsed == buff_size) flush();
 	}

 	void end() {
 		if (bufferUsed != 0) flush();
		dest->end();
 	}
};


template <typename item_t>
class virtual_sink_real_impl<item_t, 1> {
private:
	typedef virtual_source_real<item_t, 1> dest_t;
	dest_t * dest;
public:
	virtual_sink_real_impl(dest_t * d): dest(d) {};
	inline void begin(size_t count=0) {dest->begin(count);}
	inline void push(const item_t & item) {dest->push(item);}
	inline void end() {dest->end();}
};


template <typename dest_t, unsigned buff_size>
class virtual_source_impl_real
	: public virtual_source_real<typename dest_t::item_type, buff_size> {
private:
 	dest_t & dest;
public:
 	typedef typename dest_t::item_type item_type;
 	virtual_source_impl_real(dest_t & d): dest(d) {}
	virtual void begin(size_t count=0) {dest.begin(count);}
	virtual void end() {dest.end();}
 	virtual void push(const item_type * items, size_t count) {
 		for(size_t i=0; i < count; ++i)
 			dest.push(items[i]);
 	}
};

template <typename dest_t>
class virtual_source_impl_real<dest_t, 1>
	: public virtual_source_real<typename dest_t::item_type, 1> {
private:
 	dest_t & dest;
public:
 	typedef typename dest_t::item_type item_type;
 	virtual_source_impl_real(dest_t & d): dest(d) {};
	virtual void begin(size_t count=0) {dest.begin(count);}
	virtual void end() {dest.end();}
 	virtual void push(const item_type & item) {dest.push(item);}
 };



}
}

// template <typename dest_t>
// class virtual_source_real<T, 0> {
// public:
// 	virtual void push(const dest_t::item_type & item) {
		
// 	}
// };



