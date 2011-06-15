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
#include <boost/iterator/iterator_facade.hpp>
#include <boost/utility/enable_if.hpp>
#include <tpie/memory.h>

namespace tpie {

template <typename child_t, typename T, template <typename, bool> class iter_base>
class array_facade: public linear_memory_base<child_t> {
private:
	inline child_t & self() {return *static_cast<child_t*>(this);}
	inline const child_t & self() const {return *static_cast<const child_t*>(this);}
public:
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
	/// \brief Type of values containd in the array
	/////////////////////////////////////////////////////////
	typedef T value_type;

	/////////////////////////////////////////////////////////
	/// \brief Return an iterator to the i'th element of the array
	///
	/// \param i the index of the element we want an iterator to
	/// \return an iterator to the i'th element
	/////////////////////////////////////////////////////////
	iterator find(size_t idx) throw () {
		assert(idx < self().size());
		return self().get_iter(idx);
	}

	/////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the i'th element of the array
	///
	/// \param i the index of the element we want an iterator to
	/// \return a const iterator to the i'th element
	/////////////////////////////////////////////////////////
	const_iterator find(size_t idx) const throw () {
		assert(idx < self().size());
		return self().get_iter(idx);
	}
	
	/////////////////////////////////////////////////////////
	/// \brief Return the element located at the given index
	///
	/// \param i the index of the element returnd
	/////////////////////////////////////////////////////////
	T & at(size_t i) throw() {return *find(i);}

	/////////////////////////////////////////////////////////
	/// \brief Return the element located at the given index
	///
	/// \param i the index of the element returnd
	/////////////////////////////////////////////////////////
	const T & at(size_t i) const throw() {return *find(i);}
	
	/////////////////////////////////////////////////////////
	/// \brief Copy elements from another array into this.
	///
	/// Note this array is resized to the size of other
	/// \param other The array to copy from
	/// \return a reference to this array
	/////////////////////////////////////////////////////////
	child_t & operator=(const child_t & other) {
		self().resize(other.size());
		for (size_t i=0; i < self().size(); ++i) *self().get_iter(i) = *other.get_iter(i);
		return self();
	}

	/////////////////////////////////////////////////////////
	/// \brief Check if the array is empty
	///
	/// \return true if and only if size is 0
	/////////////////////////////////////////////////////////
	inline bool empty() const {return self().size() == 0;}

	/////////////////////////////////////////////////////////
	/// \brief Return a const referense to an array entry
	///
	/// \param i the index of the entry to return
	/// \return const reference to the entry
	/////////////////////////////////////////////////////////
	inline const T & operator[](size_t i) const {return at(i);}

	/////////////////////////////////////////////////////////
	/// \brief Return a referense to an array entry
	///
	/// \param i the index of the entry to return
	/// \return reference to the entry
	/////////////////////////////////////////////////////////
	inline T & operator[](size_t i) {return at(i);}


	/////////////////////////////////////////////////////////
	/// \brief Compare if the other array has the same elemens in the same order as this
	///
	/// \param other the array to compair against
	/// \return true if they are equal otherwize false
	/////////////////////////////////////////////////////////
	inline bool operator==(const child_t & other) const {
 		if (self().size() != other.size()) return false;
		for (size_t i=0;i<self().size();++i) if (*self().get_iter(i) != *other.get_iter(i)) return false;
		return true;
	}

	/////////////////////////////////////////////////////////
	/// \brief Check if the two arrayes differ
	///
	/// \param other the array to compair against
	/// \return false if they are equal otherwize true
	/////////////////////////////////////////////////////////
	inline bool operator!=(const child_t & other) const {
 		if (self().size() != other.size()) return true;
		for (size_t i=0; i<self().size(); ++i) if (*self().get_iter(i) != *other.get_iter(i)) return true;
		return false;
	}

	/////////////////////////////////////////////////////////
	/// \brief Return an iterator to the beginning of the array
	///
	/// \return an iterator tho the beginning of the array
	/////////////////////////////////////////////////////////
	inline iterator begin() {return self().get_iter(0);}

	/////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the beginning of the array
	///
	/// \return a const iterator tho the beginning of the array
	/////////////////////////////////////////////////////////
	inline const_iterator begin() const {return self().get_iter(0);}

	/////////////////////////////////////////////////////////
	/// \brief Return an iterator to the end of the array
	///
	/// \return an iterator tho the end of the array
	/////////////////////////////////////////////////////////
	inline iterator end() {return self().get_iter(self().size());}

	/////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the end of the array
	///
	/// \return a const iterator tho the end of the array
	/////////////////////////////////////////////////////////
	inline const_iterator end() const {return self().get_iter(self().size());}
	
	inline reverse_iterator rbegin() {return self().get_rev_iter(0);}
	inline const_reverse_iterator rbegin() const {return self().get_rev_iter(0);}
	inline reverse_iterator rend() {return self().get_rev_iter(self().size());}
	inline const_reverse_iterator rend() const {return self().get_rev_iter(self().size());}
};



/////////////////////////////////////////////////////////
/// \internal
/// \brief Shared implementation of array iterators
/////////////////////////////////////////////////////////
template <typename TT, bool forward>
class array_iter_base: public boost::iterator_facade<
	array_iter_base<TT, forward>,
	TT , boost::random_access_traversal_tag> {
private:
	template <typename, bool, template <typename> class > friend class array;
	friend class boost::iterator_core_access;
	template <class, bool> friend class array_iter_base;
	
	struct enabler {};
	explicit array_iter_base(TT * e): elm(e) {}
	
	inline TT & dereference() const {return * elm;}
	template <class U>
	inline bool equal(array_iter_base<U, forward> const& o) const {return elm == o.elm;}
	inline void increment() {elm += forward?1:-1;}
	inline void decrement() {elm += forward?-1:1;}
	inline void advance(size_t n) {if (forward) elm += n; else elm -= n;}
	inline ptrdiff_t distance_to(array_iter_base const & o) const {return o.elm - elm;}
	TT * elm;
public:
	array_iter_base(): elm(0) {};
	
	template <class U>
	array_iter_base(array_iter_base<U, forward> const& o, typename boost::enable_if<
			  boost::is_convertible<U*,TT*>, enabler>::type = enabler())
			: elm(o.elm) {}
};		


/////////////////////////////////////////////////////////
/// \brief A generic array with a fixed size
///
/// This is almost the same as a real new T[] array
/// but the memory managment is better
/////////////////////////////////////////////////////////
template <typename T, bool segmented=false, template <typename> class alloc_t=tpie::allocator>
class array: public array_facade<array<T, segmented, alloc_t>, T, array_iter_base> {
private:
	typedef array_facade<array<T, segmented, alloc_t>, T, array_iter_base> p_t;
	friend class array_facade<array<T, segmented, alloc_t>, T, array_iter_base>;

	T * m_elements;
	size_t m_size;
	alloc_t<T> m_allocator;

	inline typename p_t::iterator get_iter(size_t idx) {
		return typename p_t::iterator(m_elements+idx);
	}
	
	inline typename p_t::const_iterator get_iter(size_t idx) const {
		return typename p_t::const_iterator(m_elements+idx);
	}
	
	inline typename p_t::reverse_iterator get_rev_iter(size_t idx) {
		return typename p_t::reverse_iterator(m_elements+m_size-idx-1);
	}
	
	inline typename p_t::const_reverse_iterator get_rev_iter(size_t idx) const {
		return typename p_t::const_reverse_iterator(m_elements+m_size-idx-1);
	}
public:
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
	static double memory_overhead() {return 0.0;}

	/////////////////////////////////////////////////////////
	/// \brief Construct array of given size.
	///
	/// \param s The number of elements in the array
	/// \param value Each entry of the array is initialized with this value
	/////////////////////////////////////////////////////////
	array(size_type s=0, const T & value=T()): m_elements(0), m_size(0) {resize(s, value);}

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
	/// \brief Change the size of the array
	///
	/// All elements are lost.
	/// \param s the new size of the array
	/// \param elm the initialization element
	/////////////////////////////////////////////////////////
	void resize(size_t s, const T & elm=T()) {
		if (s == m_size) return;
		for (size_t i=0; i < m_size; ++i)
			m_allocator.destroy(&m_elements[i]);
		m_allocator.deallocate(m_elements, m_size);
		m_size = s;
		m_elements = s ? m_allocator.allocate(m_size) : 0;
		for (size_t i=0; i < m_size; ++i)
			m_allocator.construct(&m_elements[i], elm);
	}

	/////////////////////////////////////////////////////////
	/// \brief Return the size of the array
	///
	/// \return the size of the array
	/////////////////////////////////////////////////////////
	inline size_type size() const {return m_size;}

	/////////////////////////////////////////////////////////
	/// \brife Return a raw pointer to the array content
	/////////////////////////////////////////////////////////
	inline T * get() {return m_elements;}

	/////////////////////////////////////////////////////////
	/// \brife Return a raw pointer to the array content
	/////////////////////////////////////////////////////////
	inline const T * get() const {return m_elements;}
};

template <typename TT, bool forward>
class segmented_array_iter_base: public boost::iterator_facade<
 	segmented_array_iter_base<TT, forward>,
 	TT , boost::random_access_traversal_tag> { 
private:
	static const size_t bits=22-template_log<sizeof(TT)>::v;
	static const size_t mask = (1 << bits) -1;

	template <typename, bool, template <typename> class> friend class array;
	friend class boost::iterator_core_access;
	template <class, bool> friend class segmented_array_iter_base;

	struct enabler {};	

	inline TT & dereference() const {return m_a[m_i >> bits][m_i & mask];}
	template <class U>
	inline bool equal(segmented_array_iter_base<U, forward> const& o) const {return m_i == o.m_i;}
	inline void increment() {m_i += forward?1:-1;}
	inline void decrement() {m_i += forward?-1:1;}
	inline void advance(size_t n) {if (forward) m_i += n; else m_i -= n;}
	inline ptrdiff_t distance_to(segmented_array_iter_base const & o) const {return o.m_i - m_i;}
	segmented_array_iter_base(TT ** a, size_t i): m_a(a), m_i(i) {}

	TT **  m_a;
	size_t m_i;
public:
	segmented_array_iter_base(): m_a(0), m_i(0) {};

	template <class U>
	segmented_array_iter_base(segmented_array_iter_base<U, forward> const& o, typename boost::enable_if<
							  boost::is_convertible<U*,TT*>, enabler>::type = enabler())
		: m_a(o.m_a), m_i(o.m_i) {}
};


template <typename T, template <typename> class alloc_t>
class array<T, true, alloc_t>: public array_facade<array<T, true, alloc_t>, T, segmented_array_iter_base > {
private:
	friend class array_facade<array<T, true, alloc_t>, T, segmented_array_iter_base >;
	typedef array_facade<array<T, true, alloc_t>, T, segmented_array_iter_base > p_t;
	using p_t::at;
	static const size_t bits=p_t::iterator::bits;
	static const size_t mask=p_t::iterator::bits;

 	T ** m_a;
 	size_t m_size;
 	alloc_t<T> m_allocator;
 	alloc_t<T*> m_allocator2;
 	static size_t outerSize(size_t s) {return (s + mask)>>bits;}
	
	inline typename p_t::iterator get_iter(size_t idx) {
		return typename p_t::iterator(m_a, idx);
	}
	
	inline typename p_t::const_iterator get_iter(size_t idx) const {
		return typename p_t::const_iterator(m_a, idx);
	}
	
	// inline typename p_t::reverse_iterator get_rev_iter(size_t idx) {
	// 	return typename p_t::reverse_iterator(m_elements+m_size-idx-1);
	// }
	
	// inline typename p_t::const_reverse_iterator get_rev_iter(size_t idx) const {
	// 	return typename p_t::const_reverse_iterator(m_elements+m_size-idx-1);
	// }
public:

	/////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_coefficient()
	/// \copydetails linear_memory_structure_doc::memory_coefficient()
	/////////////////////////////////////////////////////////
	static double memory_coefficient() {
		return (double)sizeof(T) + (double)sizeof(T*)/(double)(1 << bits);
	}

	/////////////////////////////////////////////////////////
	/// \copybrief linear_memory_structure_doc::memory_overhead()
	/// \copydetails linear_memory_structure_doc::memory_overhead()
	/////////////////////////////////////////////////////////
	static double memory_overhead() {
		return sizeof(T*); //Overhead of one element in the outer tabel
	}

	/////////////////////////////////////////////////////////
	/// \brief Construct array of given size.
	///
	/// \param s The number of elements in the array
	/// \param value Each entry of the array is initialized with this value
	/////////////////////////////////////////////////////////
	array(size_type s=0, const T & value=T()): m_a(0), m_size(0) {resize(s, value);}

	/////////////////////////////////////////////////////////
	/// \brief Construct a copy of another array
	/// \param other The array to copy
	/////////////////////////////////////////////////////////
	array(const array & other): m_a(0), m_size(0) {
		resize(other.size());
		for (size_t i=0; i < m_size; ++i) 
			at(i) = other.at(i);
	}	

	/////////////////////////////////////////////////////////
	/// \brief Free up all memory used by the array
	/////////////////////////////////////////////////////////
	~array() {resize(0);}

	
	/////////////////////////////////////////////////////////
	/// \brief Change the size of the array
	///
	/// All elements are lost.
	/// \param s the new size of the array
	/// \param elm the initialization element
	/////////////////////////////////////////////////////////
	void resize(size_t size, const T & elm=T()) {
		if (m_size) {
			//Call the destructor on all the objects
			for(size_t i=0; i < m_size; ++i)
				m_allocator.destroy(&at(i));
			//Deallocate all the inner arrayes
			size_t o=outerSize(m_size);
			size_t rem=m_size;
			for(size_t i=0; rem; ++i) {
				size_t c=std::min(rem, size_t(1 << bits));
				m_allocator.deallocate(m_a[i], c);
				rem -= c;
			}
			//Deallocate the outer array
			m_allocator2.deallocate(m_a, o);
			m_a=0;
		}
		m_size=size;
		{
			size_t o=outerSize(size);
			//Allocate the outer array
			m_a = m_allocator2.allocate(o);
			//Allocate the innner arrayes
			size_t rem=m_size;
			for(size_t i=0; rem; ++i) {
				size_t c=std::min(rem, size_t(1 << bits));
				m_a[i] = m_allocator.allocate(c);
				rem -= c;
			}
			for(size_t i=0; i < m_size; ++i)
				m_allocator.construct(&at(i), elm);
		}
	}

	/////////////////////////////////////////////////////////
	/// \brief Return the size of the array
	///
	/// \return the size of the array
	/////////////////////////////////////////////////////////
	inline size_type size() const throw() {return m_size;}
};

}

namespace std {

template <typename T, template <typename> class alloc_t>
void swap(tpie::array<T, false, alloc_t> & a, tpie::array<T, false, alloc_t> & b) {

}


}

#endif //__TPIE_ARRAY_H__ 	

