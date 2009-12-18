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

#ifndef __TPIE_STREAM_CONCEPTS_H__
#define __TPIE_STREAM_CONCEPTS_H__
#include <tpie/util.h>
#include <tpie/concepts.h>

namespace tpie {
namespace stream {
namespace concepts {

template <class T>
class block_transfer_engine {
public:
	BOOST_CONCEPT_ASSERT((tpie::concepts::memory_calculatable<T>));
	BOOST_CONCEPT_USAGE(block_transfer_engine) {
		T y(true, true, 1024, 123);
		y.open("str");
		y.close();
		offset_type z = static_cast<const T*>(&y)->size();
		unused(z);
		const std::string & p = static_cast<const T*>(&y)->path();
		unused(p);
		size_type s = y.read((void *)0, (offset_type)0, (size_type)0);
		unused(s);
		y.write((void*)0, (offset_type)0, (size_type)0);
	}
};

template <class T> 
class file_stream {
public:
	typedef typename T::file_type file_type;
	typedef typename T::item_type item_type;
	
	BOOST_CONCEPT_ASSERT((tpie::concepts::memory_calculatable<T>));
	BOOST_CONCEPT_USAGE(file_stream) {
		file_type * x;
		T stream(*x, 0);
		stream.seek(1234);
		offset_type o = static_cast<const T*>(&stream)->offset();
		unused(o);
		bool h = static_cast<const T*>(&stream)->has_more();
		unused(h);
		offset_type s = static_cast<const T*>(&stream)->size();
		unused(s);
		const item_type & item = stream.read();
		stream.write(item);
		
		stream.read((item_type*)0, (item_type*)0);
		stream.write((item_type*)0, (item_type*)0);
	}
};

template <class T>
class file {
public:
	typedef typename T::item_type item_type;
	BOOST_CONCEPT_ASSERT((file_stream<typename T::stream>));
	BOOST_CONCEPT_ASSERT((tpie::concepts::memory_calculatable<T>));
	BOOST_CONCEPT_USAGE(file) {
		T f(123);
		f.open("str");
		f.close();
		offset_type o = static_cast<const T*>(&f)->size();
		unused(o);
		std::string path = static_cast<const T*>(&f)->path();
		unused(path);
	}
};

}
}
}
#endif //__TPIE_STREAM_CONCEPTS_H__
