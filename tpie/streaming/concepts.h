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
#include <boost/concept_check.hpp>

namespace tpie {
namespace streaming {

#define TPIE_UNUSED(x) (void)x

template <typename T>
struct pushable {
	typedef typename T::item_type item_type;
	
	BOOST_CONCEPT_USAGE(pushable) {
		T * x=0;
		const item_type * i=0;
		x->begin();
		x->begin(42);
		x->push(*i);
		x->end();
	}
};

template <typename T>
struct pullable {
	typedef typename T::pull_type pull_type;
	
	BOOST_CONCEPT_USAGE(pullable) {
		T * x=0;
		x->beginPull();
		x->atEnd()==true;
		const pull_type & i = x->pull();
		TPIE_UNUSED(i);
		x->endPull();
	}
};

}
}

#endif //_TPIE_STREAMING_CONCEPTS_H
