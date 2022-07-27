// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2022 The TPIE development team
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

#ifndef __TPIE_PIPELINING_FILTER_MAP_H__
#define __TPIE_PIPELINING_FILTER_MAP_H__

#include <tpie/pipelining/map.h>
#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/node_name.h>

namespace tpie::pipelining {
namespace bits {

template <typename dest_t, typename F>
class filter_map_t: public node {
private:
	F functor;
	dest_t dest;

public:
	typedef typename std::decay<typename unary_traits<F>::argument_type>::type item_type;

	filter_map_t(dest_t dest, const F & functor):
    functor(functor), dest(std::move(dest)) {
		set_name(bits::extract_pipe_name(typeid(F).name()), PRIORITY_NO_NAME);
	}

	void push(const item_type & item) {
		typename F::result_type t=f(item);
		if (t.second) dest.push(t.first);
	}
};

} //namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief A pipelining node that applies a functor to items and only keeps
/// some of them based on the functor's result.
/// \details The result of the functor must be a pair, e.g.
/// <tt>std::pair<T,bool></tt>: the second item is a boolean indiciating whether
/// item should be pushed to the next node while the first one carries the value
/// itself that is to be pushed.
/// \param functor The functor that should be applied to items
///////////////////////////////////////////////////////////////////////////////
template <typename F, typename = typename std::enable_if<bits::has_argument_type<F>::value>::type>
pipe_middle<tfactory<bits::filter_map_t, Args<F>, F> > filter_map(const F & functor) {
  return {functor};
}

} //namespace terrastream::pipelining

#endif //__TPIE_PIPELINING_FILTER_MAP_H__
