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
#ifndef __TPIE_ARRAY_H__
#define __TPIE_ARRAY_H__

///////////////////////////////////////////////////////////////////////////
/// \file array.h
/// Contains a generic internal array with known memory requirements
///////////////////////////////////////////////////////////////////////////
#include <tpie/util.h>
#include <tpie/mm.h>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/utility/enable_if.hpp>

namespace tpie {

/////////////////////////////////////////////////////////
/// \brief A generic array with a fixed size
///
/// This is almost the same as a real new T[] array
/// but the memory managment is better
/////////////////////////////////////////////////////////
template <typename T>
class array: public linear_memory_base<array<T> > {
private:
	T * m_elements;
	size_t m_size;

	/////////////////////////////////////////////////////////
	/// \internal
	/// \brief Shared implementation of array iterators
	/////////////////////////////////////////////////////////
	template <typename TT, bool forward>
	class iter_base: public boost::iterator_facade<
		iter_base<TT, forward>,
		TT , boost::random_access_traversal_tag> {
	private:
		friend class array;
		friend class boost::iterator_core_access;
		template <class, bool> friend class iter_base;

		struct enabler {};
		explicit iter_base(T * e): elm(e) {}

		inline TT & dereference() const {return * elm;}
		template <class U>
		inline bool equal(iter_base<U, forward> const& o) const {return elm == o.elm;}
		inline void increment() {elm += forward?1:-1;}
		inline void decrement() {elm += forward?-1:1;}
		inline void advance(size_t n) {if (forward) elm += n; else elm -= n;}
		inline ptrdiff_t distance_to(iter_base const & o) const {return o.elm - elm;}
		TT * elm;
	public:
		iter_base(): elm(0) {};

		template <class U>
		iter_base(iter_base<U, forward> const& o, typename boost::enable_if<
				  boost::is_convertible<U*,TT*>, enabler>::type = enabler())
			: elm(o.elm) {}
	};		

public:
	/////////////////////////////////////////////////////////
	/// \brief Type of values containd in the array
	/////////////////////////////////////////////////////////
	typedef T value_type;

	/////////////////////////////////////////////////////////
	/// \brief Iterator over a const array
	/////////////////////////////////////////////////////////
	typedef iter_base<T const, true> const_iterator;

	/////////////////////////////////////////////////////////
	/// \brief Reverse iterator over a const array
	/////////////////////////////////////////////////////////
	typedef iter_base<T const, false> const_reverse_iterator;

	/////////////////////////////////////////////////////////
	/// \brief Iterator over an array
	/////////////////////////////////////////////////////////
	typedef iter_base<T, true> iterator;

	/////////////////////////////////////////////////////////
	/// \brief Reverse iterator over an array
	/////////////////////////////////////////////////////////
	typedef iter_base<T, false> reverse_iterator;

	/////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_coefficient()
	/// \copydetails linear_memory_structure_doc::memory_coefficient()
	/////////////////////////////////////////////////////////
	static double memory_coefficient() {
		return (double)sizeof(T);
	}

	/////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_overhead()
	/// \copydetails linear_memory_structure_doc::memory_overhead()
	/////////////////////////////////////////////////////////
	static double memory_overhead() {
		return (double)sizeof(array) + MM_manager.space_overhead();
	}

	/////////////////////////////////////////////////////////
	/// \brief Construct array of given size.
	///
	/// The elements have undefined values
	/// \param s The number of elements in the array
	/////////////////////////////////////////////////////////
	array(size_type s=0): m_elements(0), m_size(0) {resize(s);}

	/////////////////////////////////////////////////////////
	/// \brief Construct array of given size.
	///
	/// \param s The number of elements in the array
	/// \param value Each entry of the array is initialized with this value
	/////////////////////////////////////////////////////////
	array(size_type s, const T & value): m_elements(0), m_size(0) {resize(s, value);}

	/////////////////////////////////////////////////////////
	/// \brief Construct a copy of another array
	/// \param other The array to copy
	/////////////////////////////////////////////////////////
	array(const array & other): m_elements(0), m_size(0) {
		resize(other.size());
		for (size_t i=0; i < m_size; ++i) m_elements[i] = other[i];
	}	

	/////////////////////////////////////////////////////////
	/// \brief Free up all memory used by the array
	/////////////////////////////////////////////////////////
	~array() {resize(0);}

	/////////////////////////////////////////////////////////
	/// \brief Copy elements from another array into this.
	///
	/// Note this array is resized to the size of other
	/// \param other The array to copy from
	/// \return a reference to this array
	/////////////////////////////////////////////////////////
	array & operator=(const array & other) {
		resize(other.size());
		for (size_t i=0; i < m_size; ++i) m_elements[i] = other[i];
		return *this;
	}
	
	/////////////////////////////////////////////////////////
	/// \brief Change the size of the array
	///
	/// All elements are lost, after resize the value of the entries
	/// is undefined
	/// \param s the new size of the array
	/////////////////////////////////////////////////////////
	void resize(size_t s) {
		if (s == m_size) return;
		delete[] m_elements;
		m_size = s;
		m_elements = s ? new T[m_size] : 0;
	}

	/////////////////////////////////////////////////////////
	/// \brief Change the size of the array
	///
	/// All elements are lost, after resize the value of the entries
	/// is initialized by a copy of elm
	/// \param s the new size of the array
	/// \param elm the initialization element
	/////////////////////////////////////////////////////////
	void resize(size_type s, const T & elm) {
		resize(s);
		for (size_type i=0; i < m_size; ++i) m_elements[i] = elm;
	}

	/////////////////////////////////////////////////////////
	/// \brief Return the size of the array
	///
	/// \return the size of the array
	/////////////////////////////////////////////////////////
	inline size_type size() const {return m_size;}

	/////////////////////////////////////////////////////////
	/// \brief Check if the array is empty
	///
	/// \return true if and only if size is 0
	/////////////////////////////////////////////////////////
	inline bool empty() const {return m_size == 0;}

	/////////////////////////////////////////////////////////
	/// \brief Return a const referense to an array entry
	///
	/// \param i the index of the entry to return
	/// \return const reference to the entry
	/////////////////////////////////////////////////////////
	inline const T & operator[](size_type i) const {
		assert(i < m_size);
		return m_elements[i];
	}

	/////////////////////////////////////////////////////////
	/// \brief Return a referense to an array entry
	///
	/// \param i the index of the entry to return
	/// \return reference to the entry
	/////////////////////////////////////////////////////////
	inline T & operator[](size_type i) {
		assert(i < m_size);
		return m_elements[i];
	}

	/////////////////////////////////////////////////////////
	/// \brief Compare if the other array has the same elemens in the same order as this
	///
	/// \param other the array to compair against
	/// \return true if they are equal otherwize false
	/////////////////////////////////////////////////////////
	inline bool operator==(const array<T> & other) const {
 		if (m_size != other.size()) return false;
		for (size_t i=0;i<m_size;++i) if (m_elements[i] != other[i]) return false;
		return true;
	}

	/////////////////////////////////////////////////////////
	/// \brief Check if the two arrayes differ
	///
	/// \param other the array to compair against
	/// \return false if they are equal otherwize true
	/////////////////////////////////////////////////////////
	inline bool operator!=(const array<T> & other) const {
 		if (m_size != other.size()) return true;
		for (size_t i=0; i<m_size; ++i) if (m_elements[i] != other[i]) return true;
		return false;
	}

	/////////////////////////////////////////////////////////
	/// \brief Return an iterator to the i'th element of the array
	///
	/// \param i the index of the element we want an iterator to
	/// \return an iterator to the i'th element
	/////////////////////////////////////////////////////////
	inline iterator find(size_type i) {return iterator(m_elements+i);}

	/////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the i'th element of the array
	///
	/// \param i the index of the element we want an iterator to
	/// \return a const iterator to the i'th element
	/////////////////////////////////////////////////////////
	inline const_iterator find(size_type i) const {return const_iterator(m_elements+i);}

	/////////////////////////////////////////////////////////
	/// \brief Return an iterator to the beginning of the array
	///
	/// \return an iterator tho the beginning of the array
	/////////////////////////////////////////////////////////
	inline iterator begin() {return iterator(m_elements);}

	/////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the beginning of the array
	///
	/// \return a const iterator tho the beginning of the array
	/////////////////////////////////////////////////////////
	inline const_iterator begin() const {return const_iterator(m_elements);}

	/////////////////////////////////////////////////////////
	/// \brief Return an iterator to the end of the array
	///
	/// \return an iterator tho the end of the array
	/////////////////////////////////////////////////////////
	inline iterator end() {return iterator(m_elements+m_size);}

	/////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the end of the array
	///
	/// \return a const iterator tho the end of the array
	/////////////////////////////////////////////////////////
	inline const_iterator end() const {return const_iterator(m_elements+m_size);}
	inline reverse_iterator rbegin() {return reverse_iterator(m_elements+m_size-1);}
	inline const_reverse_iterator rbegin() const {return const_reverse_iterator(m_elements+m_size-1);}
	inline reverse_iterator rend() {return reverse_iterator(m_elements-1);}
	inline const reverse_iterator rend() const {return const_reverse_iterator(m_elements-1);}
};

}
#endif //__TPIE_ARRAY_H__ 	

