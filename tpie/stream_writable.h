// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, The TPIE development team
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

#ifndef __TPIE_STREAM_WRITABLE_H__
#define __TPIE_STREAM_WRITABLE_H__
#include <type_traits>
#include <utility>
#include <iostream>
#include <tuple>

namespace tpie {

/*
 * We require that the item type T of a file_stream to be trivially copyable, 
 * but we relax this condition for std::pair, std::tuple
 *
 * std::pair<T1, T2> is not required by the standard to be trivially copyable
 * if T1 and T2 are trivially copyable.
 * This means that no compiler implements std::pair so it is trivially copyable in that case.
 *
 * To avoid having to write our own implementation of std::pair and require everybody who uses a file_stream to use it.
 * The standard requires that the copy constructor on the std::pair be default, so memcopying it is not undefined behaviour.
 */
template <typename T>
class is_stream_writable_override {
private:
  template <typename TT>
  static char magic(typename TT::trivially_copyable*);

  template <typename TT>
  static char magic(typename TT::stream_writable*);

  template <typename TT>
  static long magic(...);
public:
  static bool const value=sizeof(magic<T>((std::true_type*)nullptr))==sizeof(char);
};

template <typename ... TT>
struct is_stream_writable {};

template <>
struct is_stream_writable<>:  std::integral_constant<bool, true> {};

template <typename T1, typename T2, typename ... TT>
struct is_stream_writable<T1, T2, TT...>: 
  std::integral_constant<bool, is_stream_writable<T1>::value && is_stream_writable<T2, TT...>::value> {};

template <typename T>
struct is_stream_writable<T> :
  std::integral_constant<bool, std::is_trivially_copyable<T>::value || is_stream_writable_override<T>::value> {};

template <typename T1, typename T2>
struct is_stream_writable<std::pair<T1, T2>> : is_stream_writable<T1, T2> {};

template <typename ... TT>
struct is_stream_writable<std::tuple<TT...>> : is_stream_writable<TT...> {};

} //namespace tpie
#endif //__TPIE_STREAM_WRITABLE_H__
