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

#include <boost/iterator/iterator_facade.hpp>
#include <vector>
#include <tpie/array.h>

namespace tpie {

template <typename T>
class array_view_base {
private:
	T * m_start;
	T * m_end;
public:
	class iterator: public boost::iterator_facade<iterator, T, boost::random_access_traversal_tag> {
	private:
		friend class array_view_base;
		friend class boost::iterator_core_access;
		explicit iterator(T * e): elm(e) {}
		inline T & dereference() const {return * elm;}
		inline bool equal(iterator const& o) const {return elm == o.elm;}
		inline void increment() {++elm;}
		inline void decrement() {--elm;}
		inline void advance(size_t n) {elm += n;}
		inline ptrdiff_t distance_to(iterator const & o) const {return o.elm - elm;}
		T * elm;
	public:
		iterator(): elm(0) {};
	};		
	

	array_view_base(T * s, T * e): m_start(s), m_end(e) {}
	
	/////////////////////////////////////////////////////////
	/// \brief Type of values containd in the array
	/////////////////////////////////////////////////////////
	typedef T value_type;
	
	/////////////////////////////////////////////////////////
	/// \brief Return an iterator to the i'th element of the array
	///
	/// \param i the index of the element we want an iterator to
	/// \return an iterator to the i'th element
	/////////////////////////////////////////////////////////
	iterator find(size_t idx) const throw () {
		assert(idx <= size());
		return iterator(m_start + idx);
	}

	/////////////////////////////////////////////////////////
	/// \brief Return the element located at the given index
	///
	/// \param i the index of the element returnd
	/////////////////////////////////////////////////////////
	T & at(size_t i) const throw() {
		assert(i < size());
		return *find(i);
	}

	/////////////////////////////////////////////////////////
	/// \brief Check if the array is empty
	///
	/// \return true if and only if size is 0
	/////////////////////////////////////////////////////////
	inline bool empty() const {return m_end == m_start;}

	inline size_t size() const {return m_end - m_start;}

	/////////////////////////////////////////////////////////
	/// \brief Return a referense to an array entry
	///
	/// \param i the index of the entry to return
	/// \return reference to the entry
	/////////////////////////////////////////////////////////
	inline T & operator[](size_t i) const {
		assert(i < size());
		return at(i);
	}

	/////////////////////////////////////////////////////////
	/// \brief Compare if the other array has the same elemens in the same order as this
	///
	/// \param other the array to compair against
	/// \return true if they are equal otherwize false
	/////////////////////////////////////////////////////////
	inline bool operator==(const array_view_base & other) const {
 		if (size() != other.size()) return false;
		for (size_t i=0; i < size(); ++i) if (at(i) != other.at(i)) return false;
		return true;
	}

	/////////////////////////////////////////////////////////
	/// \brief Check if the two arrayes differ
	///
	/// \param other the array to compair against
	/// \return false if they are equal otherwize true
	/////////////////////////////////////////////////////////
	inline bool operator!=(const array_view_base & other) const {
		if (size() != other.size()) return true;
		for (size_t i=0; i< size(); ++i) if (at(i) != other.at(i)) return true;
		return false;
	}

	/////////////////////////////////////////////////////////
	/// \brief Return an iterator to the beginning of the array
	///
	/// \return an iterator tho the beginning of the array
	/////////////////////////////////////////////////////////
	inline iterator begin() const {return iterator(m_start);}

	/////////////////////////////////////////////////////////
	/// \brief Return an iterator to the end of the array
	///
	/// \return an iterator tho the end of the array
	/////////////////////////////////////////////////////////
	inline iterator end() const {return iterator(m_end);}

	/////////////////////////////////////////////////////////
	/// \brief return the first element in the array
	/////////////////////////////////////////////////////////
	inline T & front() const {return *m_start;}

	/////////////////////////////////////////////////////////
	/// \brief return the last element in the array
	/////////////////////////////////////////////////////////
	inline T & back() const {return *(m_end-1);}
};

template <typename T>
class array_view: public array_view_base<T> {
public:
	inline array_view(const array_view & o): 
		array_view_base<T>(o) {}
	
	inline array_view(std::vector<T> & v): 
		array_view_base<T>(&*(v.begin()), &(*v.end())) {}
	
	inline array_view(tpie::array<T> & v): 
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
