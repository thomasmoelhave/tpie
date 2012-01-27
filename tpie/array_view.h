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

#ifndef __ARRAY_VIEW_H__
#define  __ARRAY_VIEW_H__

#include <vector>
#include <tpie/array.h>
#include <tpie/array_view_base.h>
#include <tpie/internal_vector.h>

namespace tpie {


template <typename T>
class array_view: public array_view_base<T> {
public:
	inline array_view(const array_view & o): 
		array_view_base<T>(o) {}
	
	inline array_view(std::vector<T> & v): 
		array_view_base<T>(&*(v.begin()), &(*v.end())) {}
	
	inline array_view(tpie::array<T> & v): 
		array_view_base<T>(&*(v.begin()), &(*v.end())) {}

	inline array_view(tpie::internal_vector<T> & v): 
		array_view_base<T>(&*(v.begin()), &(*v.end())) {}
	
	inline array_view(std::vector<T> & v, size_t start, size_t end): 
		array_view_base<T>(&v[start], &v[end]) {}
	
	inline array_view(tpie::array<T> & v, size_t start, size_t end): 
		array_view_base<T>(&v[start], &v[end]) {}
	
	inline array_view(typename std::vector<T>::iterator start, typename std::vector<T>::iterator end): 
		array_view_base<T>(&*start, &*end) {}
	
	inline array_view(typename array<T>::iterator start, typename array<T>::iterator end): 
		array_view_base<T>(&*start, &*end) {}
	
	inline array_view(T * start, T * end):
		array_view_base<T>(start, end) {}
	
	inline array_view(T * start, size_t size): 
		array_view_base<T>(start, start+size) {}
};

template <typename T>
class array_view<const T>: public array_view_base<const T> {
public:
	inline array_view(array_view<T> o): 
		array_view_base<const T>(&*(o.begin()), &*(o.end())) {}

	inline array_view(const std::vector<T> & v): 
		array_view_base<const T>(&*(v.begin()), &*(v.end())) {}
	
	inline array_view(const tpie::array<T> & v): 
		array_view_base<const T>(&*(v.begin()), &*(v.end())) {}
	
	inline array_view(const tpie::internal_vector<T> & v): 
		array_view_base<const T>(&*(v.begin()), &(*v.end())) {}

	inline array_view(const std::vector<T> & v, size_t start, size_t end): 
		array_view_base<const T>(&v[start], &v[end]) {}
	
	inline array_view(const tpie::array<T> & v, size_t start, size_t end): 
		array_view_base<const T>(&v[start], &v[end]) {}
	
	inline array_view(typename std::vector<T>::const_iterator start, typename std::vector<T>::const_iterator end): 
		array_view_base<const T>(&*start, &*end) {}
	
	inline array_view(typename array<T>::const_iterator start, typename array<T>::const_iterator end): 
		array_view_base<const T>(&*start, &*end) {}
	
	inline array_view(const T * start, const T * end): 
		array_view_base<const T>(start, end) {}
	
	inline array_view(const T * start, size_t size): 
		array_view_base<const T>(start, start+size) {}
};

} //namespace tpie

#endif // __ARRAY_VIEW_H__
