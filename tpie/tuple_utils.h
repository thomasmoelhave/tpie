// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2018, The TPIE development team
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

#include <tuple>
#include <type_traits>

namespace tpie {

template <typename F, typename Tuple, size_t I = 0>
std::enable_if_t<I == std::tuple_size<typename std::remove_reference<Tuple>::type>::value>
tuple_for_each(F, Tuple &&) {}

///////////////////////////////////////////////////////////////////////////////
/// \brief Calls a functor on element of a std::tuple in order
/// \param f The functor that should be called
/// \param t The tuple
///////////////////////////////////////////////////////////////////////////////
template <typename F, typename Tuple, size_t I = 0>
std::enable_if_t<I != std::tuple_size<typename std::remove_reference<Tuple>::type>::value>
tuple_for_each(F f, Tuple && t) {
	f(std::get<I>(t));
	tuple_for_each<F, Tuple, I + 1>(f, t);
}

} //namespace tpie
