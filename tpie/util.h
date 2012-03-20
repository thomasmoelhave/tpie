// -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, 2012, The TPIE development team
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

///////////////////////////////////////////////////////////////////////////////
/// \file util.h  Miscellaneous utility functions
///////////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_UTIL_H__
#define __TPIE_UTIL_H__

#include <tpie/portability.h>
#include <tpie/types.h>
#include <cmath>
#include <string>
#include <cstdio>
namespace tpie {

///////////////////////////////////////////////////////////////////////////
/// \brief Ignore an unused variable warning
/// \param x The variable that we are well aware is not beeing useod
///////////////////////////////////////////////////////////////////////////
template <typename T>
inline void unused(const T & x) {(void)x;}

template <typename T> struct sign {typedef T type;};
template <> struct sign<uint8_t> {typedef int8_t type;};
template <> struct sign<uint16_t> {typedef int16_t type;};
template <> struct sign<uint32_t> {typedef int32_t type;};
template <> struct sign<uint64_t> {typedef int64_t type;};

template <typename T> struct unsign {typedef T type;};
template <> struct unsign<int8_t> {typedef uint8_t type;};
template <> struct unsign<int16_t> {typedef uint16_t type;};
template <> struct unsign<int32_t> {typedef uint32_t type;};
template <> struct unsign<int64_t> {typedef uint64_t type;};

#ifdef _WIN32
const char directory_delimiter = '\\';
#else
const char directory_delimiter = '/';
#endif

///////////////////////////////////////////////////////////////////////////
/// Any internal memory datastructur whos memory usage is linear
/// in the numebr of elements.
/// The structure must implement memory_cooeficient and
/// memory_overhead
///////////////////////////////////////////////////////////////////////////
template <typename child_t> 
struct linear_memory_base {

	///////////////////////////////////////////////////////////////////////////
	// Return the number of bytes required to create a datatstucture supporting 
	// a given number of elements
	// \param size The number of elements to support
	// \return The abount of memory required in bytes
	///////////////////////////////////////////////////////////////////////////
	inline static stream_size_type memory_usage(stream_size_type size) {
		return static_cast<stream_size_type>(
			floor(size * child_t::memory_coefficient() + child_t::memory_overhead()));
	}

	///////////////////////////////////////////////////////////////////////////
	// Return the maximum number of elements that can be contained in 
	// in the stucture when it is allowed to fill a given number of bytes
	// \param memory The number of bytes the structure is allowed to occupie
	// \return The number of elements that will fit in the structure
	///////////////////////////////////////////////////////////////////////////
	inline static memory_size_type memory_fits(memory_size_type memory) {
		return static_cast<memory_size_type>(
			floor((memory - child_t::memory_overhead()) / child_t::memory_coefficient()));
	}
};

template <int t>
struct template_log {
	static const size_t v=1+template_log< t/2 >::v;
};

template <>
struct template_log<1> {
	static const size_t v=1;
};


/////////////////////////////////////////////////////////
/// \brief Restore heap invariants after the first element 
/// has been replaced by some other olement
/////////////////////////////////////////////////////////
template <typename T, typename C>
void pop_and_push_heap(T a, T b, C lt) {
	size_t i=0;
	size_t n=(b-a);
	while (true) {
		size_t c=2*i+1;
		if (c+1 >= n) {
			if (c < n && lt(*(a+i), *(a+c)))
				std::swap(*(a+c), *(a+i));
			break;
		}
		if (lt(*(a+c+1), *(a+c))) {
			if (lt(*(a+i), *(a+c))) {
				std::swap(*(a+c), *(a+i));
				i=c;
			} else break;
		} else {
			if (lt(*(a+i), *(a+c+1))) {
				std::swap(*(a+c+1), *(a+i));
				i=c+1;
			} else break;
		}
	}
}

/////////////////////////////////////////////////////////
/// \brief Restore heap invariants after the first element 
/// has been replaced by some other olement
/////////////////////////////////////////////////////////
template <typename T>
void pop_and_push_heap(T a, T b) {
	pop_and_push_heap(a,b, std::less<typename T::value_type>());
}


/////////////////////////////////////////////////////////
/// A binary functor object with the arguments swapped
/////////////////////////////////////////////////////////
template <typename T>
struct binary_argument_swap: public std::binary_function<typename T::second_argument_type, 
														 typename T::first_argument_type, 
														 typename T::result_type> {
	T i;
	binary_argument_swap(T & _): i(_) {}
	inline bool operator()(const typename T::second_argument_type & x, 
						   const typename T::first_argument_type & y) {
		return i(y,x);
	}
};


inline void atomic_rename(const std::string & src, const std::string & dst) {
	//Note according to posix rename is atomic..
	//On windows it is probably not
#ifndef _WIN32
	if (rename(src.c_str(), dst.c_str()) != 0)
		throw std::runtime_error("Atomic rename failed");
#else
	//TODO use MoveFileTransacted on vista or newer
	if (!MoveFileEx(src.c_str(), dst.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		throw std::runtime_error("Atomic rename failed");
#endif
}


} //namespace tpie

#endif //__TPIE_UTIL_H__
