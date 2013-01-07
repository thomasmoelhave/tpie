// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2010, The TPIE development team
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

#ifndef TPIE_SERIALIZATION2_H
#define TPIE_SERIALIZATION2_H

///////////////////////////////////////////////////////////////////////////
/// \file tpie/serialization.h Binary serialization and unserialization.
///////////////////////////////////////////////////////////////////////////

#include <tpie/config.h>
#include <tpie/portability.h>
#include <typeinfo>
#include <boost/type_traits/is_pod.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/utility/enable_if.hpp>
#include <tpie/is_simple_iterator.h>

namespace tpie_serialize {}

namespace tpie {

namespace bits {
using namespace tpie_serialize;

template <typename D, typename T, 
		  bool is_simple_itr=tpie::is_simple_iterator<T>::value,
		  bool is_pod=boost::is_pod<typename std::iterator_traits<T>::value_type>::value,
		  bool is_pointer=boost::is_pointer<typename std::iterator_traits<T>::value_type>::value>
struct array_encode_magic {
	//using namespace tpie_serialize;
    void operator()(D & dst, T start, T end) {
		for (T i=start; i != end; ++i) tp_serialize(dst, *i);
    }
};

template <typename D, typename T>
struct array_encode_magic<D, T, true, true, false> {
    void operator()(D & d, T start, T end) {
		d.write((const char*)(&(*start)), (end-start)*sizeof(T));
    }
};

template <typename D, typename T, 
		  bool is_simple_itr=tpie::is_simple_iterator<T>::value,
		  bool is_pod=boost::is_pod<typename std::iterator_traits<T>::value_type>::value,
		  bool is_pointer=boost::is_pointer<typename std::iterator_traits<T>::value_type>::value>
struct array_decode_magic {
	//using namespace tpie_serialize;
    void operator()(D & dst, T start, T end) {
		for (T i=start; i != end; ++i) tp_unserialize(dst, *i);
    }
};

template <typename D, typename T>
struct array_decode_magic<D, T, true, true, false> {
    void operator()(D & d, T start, T end) {
		d.read((char *)(&(*start)), (end-start)*sizeof(T));
    }
};

struct counter {
	size_t size;
	counter(): size(0) {}
	void write(void *, size_t s) {size += s;}
};

};

template <class D, typename T>
void serialize(D & dst, const T & v) {
	using namespace tpie_serialize;
	tp_serialize(dst, v);
}
template <class D, typename T>
void serialize(D & dst, T start, T end) {
	using namespace tpie_serialize;
	bits::array_encode_magic<D, T> magic;
	magic(dst, start, end);
}

template <class S, typename T>
void unserialize(S & src, T & v) {
	using namespace tpie_serialize;
	tp_unserialize(src, v);
}

template <class D, typename T>
void unserialize(D & dst, T start, T end) {
	using namespace tpie_serialize;
	bits::array_decode_magic<D, T> magic;
	magic(dst, start, end);
}

template <typename T>
size_t serialized_size(const T & v) {
	bits::counter c;
	serialize(c, v);
	return c.size;
}


};

namespace tpie_serialize {

template <class D, typename T>
void tp_serialize(D & dst, const T & v,
				  typename boost::enable_if<boost::is_pod<T> >::type *_=0,
				  typename boost::disable_if<boost::is_pointer<T> >::type *__=0) {
	tpie::unused(_);
	tpie::unused(__);
	dst.write((const char *)&v, sizeof(T));
}

template <class S, typename T>
void tp_unserialize(S & src, T & v,
					typename boost::enable_if<boost::is_pod<T> >::type *_=0,
					typename boost::disable_if<boost::is_pointer<T> >::type *__=0) {
	tpie::unused(_);
	tpie::unused(__);
	src.read((char *)&v, sizeof(T));
}

template <class D, typename T, typename alloc_t>
void tp_serialize(D & dst, const std::vector<T, alloc_t> & v) {
	tpie::serialize(dst, v.size());
    tpie::serialize(dst, v.begin(), v.end());
}

template <class S, typename T, typename alloc_t>
void tp_unserialize(S & src, std::vector<T, alloc_t> & v) {
	typename std::vector<T>::size_type s;
	tpie::unserialize(src, s);
	v.resize(s);
    tpie::unserialize(src, v.begin(), v.end());
}

template <class D, typename T>
void tp_serialize(D & dst, const std::basic_string<T> & v) {
	tpie::serialize(dst, v.size());
    tpie::serialize(dst, v.begin(), v.end());
}

template <class S, typename T>
void tp_unserialize(S & src, std::basic_string<T> & v) {
	typename std::basic_string<T>::size_type s;
	tpie::unserialize(src, s);
	v.resize(s);
    tpie::unserialize(src, v.begin(), v.end());
}

} //namespace tpie_serialize

#endif // TPIE_SERIALIZATION2_H
