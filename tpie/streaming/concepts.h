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
#ifndef _TPIE_STREAMING_CONCEPTS_H
#define _TPIE_STREAMING_CONCEPTS_H
#include <tpie/util.h>
#include <tpie/concepts.h>

namespace tpie {
namespace streaming {
namespace concepts {

template <typename T>
struct memory_managable {
	BOOST_CONCEPT_USAGE(memory_managable) {
		T * x=0;
		memory_base * y = static_cast<memory_base *>(x);
		unused(y);
	}
};

template <typename T>
struct pushable {
	typedef typename T::item_type item_type;
	typedef typename T::begin_data_type begin_data_type;
	typedef typename T::end_data_type end_data_type;
	 
	BOOST_CONCEPT_USAGE(pushable) {
 		T * x=0;
 		const item_type * i=0;
 		x->begin();
 		x->begin((stream_size_type)42);
 		x->begin((stream_size_type)42,(begin_data_type*)0);
 		x->push(*i);
 		x->end();
 		x->end((end_data_type*)0);
	}
};


template <typename T>
struct pullable {
	typedef typename T::pull_type pull_type;
	
	BOOST_CONCEPT_USAGE(pullable) {
		T * x=0;
		x->pull_begin();
		x->pull_begin((stream_size_type*)0);
		x->can_pull()==true;
		const pull_type & i = x->pull();
		unused(i);
		x->pull_end();
	}
};

}
}
}

#endif //_TPIE_STREAMING_CONCEPTS_H
