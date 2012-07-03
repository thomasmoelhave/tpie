// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2010, 2012, The TPIE development team
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
/// Generic internal array with known memory requirements.
///////////////////////////////////////////////////////////////////////////
#include <tpie/util.h>
#include <boost/type_traits/is_pod.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/utility/enable_if.hpp>
#include <tpie/memory.h>
#include <tpie/array_view_base.h>

namespace tpie {

///////////////////////////////////////////////////////////////////////////////
/// \internal
///////////////////////////////////////////////////////////////////////////////

template <typename child_t, typename T, template <typename, bool> class iter_base>
class array_facade: public linear_memory_base<child_t> {
public:
	/** \brief Iterator over a const array */
	typedef iter_base<T const, true> const_iterator;

	/** \brief Reverse iterator over a const array */
	typedef iter_base<T const, false> const_reverse_iterator;

	/** \brief Iterator over an array */
	typedef iter_base<T, true> iterator;

	/** \brief Reverse iterator over an array */
	typedef iter_base<T, false> reverse_iterator;

	/** \brief Type of values containd in the array */
	typedef T value_type;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return an iterator to the i'th element of the array.
	///
	/// \param idx The index of the element we want an iterator to.
	/// \return An iterator to the i'th element.
	///////////////////////////////////////////////////////////////////////////
	iterator find(size_t idx) throw () {
		assert(idx <= self().size());
		return self().get_iter(idx);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the i'th element of the array.
	///
	/// \param idx The index of the element we want an iterator to.
	/// \return A const iterator to the i'th element.
	///////////////////////////////////////////////////////////////////////////
	const_iterator find(size_t idx) const throw () {
		assert(idx <= self().size());
		return self().get_iter(idx);
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the element located at the given index.
	///
	/// \param i The index of the element returned.
	///////////////////////////////////////////////////////////////////////////
	T & at(size_t i) throw() {
		assert(i < self().size());
		return *find(i);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc tpie::array_facade::at(size_t i)
	///////////////////////////////////////////////////////////////////////////
	const T & at(size_t i) const throw() {
		assert(i < self().size());
		return *find(i);
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Copy elements from another array into this.
	///
	/// Note: This array is resized to the size of other.
	///
	/// \param other The array to copy from.
	/// \return A reference to this array.
	///////////////////////////////////////////////////////////////////////////
	child_t & operator=(const child_t & other) {
		self().resize(other.size());
		for (size_t i=0; i < self().size(); ++i) *self().get_iter(i) = *other.get_iter(i);
		return self();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Check if the array is empty.
	///
	/// \return True if and only if size is 0.
	///////////////////////////////////////////////////////////////////////////
	inline bool empty() const {return self().size() == 0;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a const reference to an array entry.
	///
	/// \param i The index of the entry to return.
	/// \return Const reference to the entry.
	///////////////////////////////////////////////////////////////////////////
	inline const T & operator[](size_t i) const {
		assert(i < self().size());
		return self().at(i);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a reference to an array entry.
	///
	/// \param i The index of the entry to return.
	/// \return Reference to the entry.
	///////////////////////////////////////////////////////////////////////////
	inline T & operator[](size_t i) {
		assert(i < self().size());
		return self().at(i);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Compare if the other array has the same elements in the same
	/// order as this.
	///
	/// \param other The array to compare against.
	/// \return True if they are equal otherwise false.
	///////////////////////////////////////////////////////////////////////////
	inline bool operator==(const child_t & other) const {
 		if (self().size() != other.size()) return false;
		for (size_t i=0;i<self().size();++i) if (*self().get_iter(i) != *other.get_iter(i)) return false;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Check if two arrays differ.
	///
	/// \param other The array to compare against.
	/// \return False if they are equal; otherwise true.
	///////////////////////////////////////////////////////////////////////////
	inline bool operator!=(const child_t & other) const {
 		if (self().size() != other.size()) return true;
		for (size_t i=0; i<self().size(); ++i) if (*self().get_iter(i) != *other.get_iter(i)) return true;
		return false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return an iterator to the beginning of the array.
	///
	/// \return An iterator to the beginning of the array.
	///////////////////////////////////////////////////////////////////////////
	inline iterator begin() {return self().get_iter(0);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the beginning of the array.
	///
	/// \return A const iterator to the beginning of the array.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator begin() const {return self().get_iter(0);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return an iterator to the end of the array.
	///
	/// \return An iterator to the end of the array.
	///////////////////////////////////////////////////////////////////////////
	inline iterator end() {return self().get_iter(self().size());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a const iterator to the end of the array.
	///
	/// \return A const iterator to the end of the array.
	///////////////////////////////////////////////////////////////////////////
	inline const_iterator end() const {return self().get_iter(self().size());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the first element in the array.
	///////////////////////////////////////////////////////////////////////////
	inline const T & front() const {return at(0);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the first element in the array.
	///////////////////////////////////////////////////////////////////////////
	inline T & front() {return at(0);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the last element in the array.
	///////////////////////////////////////////////////////////////////////////
	inline const T & back() const {return at(self().size()-1);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the last element in the array.
	///////////////////////////////////////////////////////////////////////////
	inline T & back() {return at(self().size()-1);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Reverse iterator to beginning of reverse sequence.
	///////////////////////////////////////////////////////////////////////////
	inline reverse_iterator rbegin() {return self().get_rev_iter(0);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Const reverse iterator to beginning of reverse sequence.
	///////////////////////////////////////////////////////////////////////////
	inline const_reverse_iterator rbegin() const {return self().get_rev_iter(0);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Reverse iterator to end of reverse sequence.
	///////////////////////////////////////////////////////////////////////////
	inline reverse_iterator rend() {return self().get_rev_iter(self().size());}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Const reverse iterator to end of reverse sequence.
	///////////////////////////////////////////////////////////////////////////
	inline const_reverse_iterator rend() const {return self().get_rev_iter(self().size());}

private:
	inline child_t & self() {return *static_cast<child_t*>(this);}
	inline const child_t & self() const {return *static_cast<const child_t*>(this);}
};

///////////////////////////////////////////////////////////////////////////////
/// \internal
/// \brief Shared implementation of array iterators.
///////////////////////////////////////////////////////////////////////////////
template <typename TT, bool forward>
class array_iter_base: public boost::iterator_facade<
	array_iter_base<TT, forward>,
	TT , boost::random_access_traversal_tag> {
private:
	template <typename, bool> friend class array_base;
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
	///////////////////////////////////////////////////////////////////////////
	/// \brief Default constructor.
	///////////////////////////////////////////////////////////////////////////
	array_iter_base(): elm(0) {};
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Copy constructor.
	/// We use boost::enable_if to allow copying an iterator with a more
	/// specific item_type to an iterator with a more general item_type.
	///////////////////////////////////////////////////////////////////////////
	template <class U>
	array_iter_base(array_iter_base<U, forward> const& o, typename boost::enable_if<
			  boost::is_convertible<U*,TT*>, enabler>::type = enabler())
			: elm(o.elm) {}
};		

///////////////////////////////////////////////////////////////////////////////
/// \brief A generic array with a fixed size.
///
/// This is almost the same as a real C-style T array but the memory management
/// is better.
///
/// Do not instantiate this class directly. Instead, use tpie::array or
/// tpie::segmented_array.
///
/// \tparam T The type of element to contain.
/// \tparam segmented Whether to use segmented arrays.
/// \tparam alloc_t Allocator.
///////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)
template <typename C>
struct trivial_same_size {
	char c[sizeof(C)];
};
#pragma pack(pop)

template <typename T, bool segmented=false>
class array_base: public array_facade<array_base<T, segmented>, T, array_iter_base> {
private:
	typedef array_facade<array_base<T, segmented>, T, array_iter_base> p_t;
	friend class array_facade<array_base<T, segmented>, T, array_iter_base>;

	T * m_elements;
	size_t m_size;

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
	///////////////////////////////////////////////////////////////////////////
	/// \copydoc tpie::linear_memory_structure_doc::memory_coefficient()
	///////////////////////////////////////////////////////////////////////////
	static double memory_coefficient() {
		return (double)sizeof(T);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc tpie::linear_memory_structure_doc::memory_overhead()
	///////////////////////////////////////////////////////////////////////////
	static double memory_overhead() {return sizeof(array_base);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct array of given size.
	///
	/// \param s The number of elements in the array.
	/// \param value Each entry of the array is initialized with this value.
	///////////////////////////////////////////////////////////////////////////
	array_base(size_type s, const T & value): m_elements(0), m_size(0), m_tss_used(false) {resize(s, value);}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct array of given size.
	///
	/// \param s The number of elements in the array.
	///////////////////////////////////////////////////////////////////////////
	array_base(size_type s=0): m_elements(0), m_size(0), m_tss_used(false) {resize(s);}

	/////////////////////////////////////////////////////////
	/// \brief Construct a copy of another array.
	/// \param other The array to copy.
	/////////////////////////////////////////////////////////
	array_base(const array_base & other): m_elements(0), m_size(other.m_size) {
		if (other.size() == 0) return;
		m_elements = m_size ? reinterpret_cast<T*>(tpie_new_array<trivial_same_size<T> >(m_size)) : 0;
		m_tss_used = true;
		std::uninitialized_copy(other.m_elements+0, other.m_elements+m_size, m_elements+0);
	}	

	///////////////////////////////////////////////////////////////////////////
	/// \brief Free up all memory used by the array.
	///////////////////////////////////////////////////////////////////////////
	~array_base() {resize(0);}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Change the size of the array.
	///
	/// All elements are lost.
	///
	/// Memory manager MUST be initialized at this point unless s == 0.
	///
	/// \param size The new size of the array.
	/// \param elm The initialization element.
	///////////////////////////////////////////////////////////////////////////
	void resize(size_t size, const T & elm) {
		if (size != m_size) {
			destruct_and_dealloc();
			m_size = size;

			m_elements = size ? reinterpret_cast<T*>(tpie_new_array<trivial_same_size<T> >(m_size)) : 0;
			m_tss_used = true;

			// call copy constructors manually
			std::uninitialized_fill(m_elements+0, m_elements+m_size, elm);
		} else {
			std::fill(m_elements+0, m_elements+m_size, elm);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Swap two arrays.
	///////////////////////////////////////////////////////////////////////////
	void swap(array_base & other) {
		std::swap(m_elements, other.m_elements);
		std::swap(m_size, other.m_size);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Change the size of the array
	///
	/// All elements are lost.
	///
	/// Memory manager MUST be initialized at this point unless s == 0.
	///
	/// \param s The new size of the array.
	///////////////////////////////////////////////////////////////////////////
	void resize(size_t s) {
		destruct_and_dealloc();
		m_size = s;
		m_elements = m_size ? tpie_new_array<T>(m_size) : 0;
		m_tss_used = false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return the size of the array.
	///
	/// \return The size of the array.
	///////////////////////////////////////////////////////////////////////////
	inline size_type size() const {return m_size;}

	inline T & at(size_t i) { return m_elements[i]; }
	inline const T & at(size_t i) const { return m_elements[i]; }

protected:
	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a raw pointer to the array content.
	///////////////////////////////////////////////////////////////////////////
	inline T * __get() {return m_elements;}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a raw pointer to the array content.
	///////////////////////////////////////////////////////////////////////////
	inline const T * __get() const {return m_elements;}

private:
	inline void destruct_and_dealloc() {
		if (!m_tss_used) {
			// calls destructors
			tpie_delete_array(m_elements, m_size);
			return;
		}

		// call destructors manually
		for (size_t i = 0; i < m_size; ++i) {
			m_elements[i].~T();
		}
		tpie_delete_array(reinterpret_cast<trivial_same_size<T>*>(m_elements), m_size);
	}

	// did we allocate m_elements as a trivial_same_size<T> *?
	bool m_tss_used;
};

///////////////////////////////////////////////////////////////////////////////
/// \internal
///////////////////////////////////////////////////////////////////////////////

template <typename TT, bool forward>
class segmented_array_iter_base: public boost::iterator_facade<
 	segmented_array_iter_base<TT, forward>,
 	TT , boost::random_access_traversal_tag> { 
private:
	static const size_t bits=22-template_log<sizeof(TT)>::v;
	static const size_t mask = (1 << bits) -1;

	template <typename, bool> friend class array_base;
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


	///////////////////////////////////////////////////////////////////////////
	/// \copydoc array_iter_base::array_iter_base(array_iter_base<U, forward> const &o, typename boost::enable_if<boost::is_convertible<U *, TT *>, enabler>::type)
	///////////////////////////////////////////////////////////////////////////
	template <class U>
	segmented_array_iter_base(segmented_array_iter_base<U, forward> const& o, typename boost::enable_if<
							  boost::is_convertible<U*,TT*>, enabler>::type = enabler())
		: m_a(o.m_a), m_i(o.m_i) {}
};

///////////////////////////////////////////////////////////////////////////////
/// \internal
/// \brief Segmented array implementation.
///
/// Specializes array_base for segmented == true.
///////////////////////////////////////////////////////////////////////////////

template <typename T>
class array_base<T, true>: public array_facade<array_base<T, true>, T, segmented_array_iter_base > {
private:
	friend class array_facade<array_base<T, true>, T, segmented_array_iter_base >;
	typedef array_facade<array_base<T, true>, T, segmented_array_iter_base > p_t;
	using p_t::at;
	static const size_t bits=p_t::iterator::bits;
	static const size_t mask=p_t::iterator::mask;

 	T ** m_a;
 	size_t m_size;
 	allocator<T> m_allocator;
 	allocator<T*> m_allocator2;
 	static size_t outerSize(size_t s) {return (s + mask)>>bits;}
	
	inline typename p_t::iterator get_iter(size_t idx) {
		return typename p_t::iterator(m_a, idx);
	}
	
	inline typename p_t::const_iterator get_iter(size_t idx) const {
		return typename p_t::const_iterator((const T **)m_a, idx);
	}
	
	// inline typename p_t::reverse_iterator get_rev_iter(size_t idx) {
	// 	return typename p_t::reverse_iterator(m_elements+m_size-idx-1);
	// }
	
	// inline typename p_t::const_reverse_iterator get_rev_iter(size_t idx) const {
	// 	return typename p_t::const_reverse_iterator(m_elements+m_size-idx-1);
	// }
public:

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc array_base::memory_coefficient()
	///////////////////////////////////////////////////////////////////////////
	static double memory_coefficient() {
		return (double)sizeof(T) + (double)sizeof(T*)/(double)(1 << bits);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc array_base::memory_overhead()
	///////////////////////////////////////////////////////////////////////////
	static double memory_overhead() {
		return sizeof(T*); //Overhead of one element in the outer table
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc array_base::array_base(size_type, const T &)
	///////////////////////////////////////////////////////////////////////////
	array_base(size_type s=0, const T & value=T()): m_a(0), m_size(0) {resize(s, value);}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc array_base::array_base(const array_base &)
	///////////////////////////////////////////////////////////////////////////
	array_base(const array_base & other): m_a(0), m_size(0) {
		resize(other.size());
		for (size_t i=0; i < m_size; ++i) 
			at(i) = other.at(i);
	}	

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc array_base::~array_base()
	///////////////////////////////////////////////////////////////////////////
	~array_base() {resize(0);}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc array_base::swap(array_base &)
	///////////////////////////////////////////////////////////////////////////
	void swap(array_base & other) {
		std::swap(m_a, other.m_a);
		std::swap(m_size, other.m_size);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc array_base::resize(size_t, const T &)
	///////////////////////////////////////////////////////////////////////////
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
		if (m_size) {
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

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc array_base::size()
	///////////////////////////////////////////////////////////////////////////
	inline size_type size() const throw() {return m_size;}
};

/** Determine if we should use the segmented_array implementation. On 64-bit
 * systems the address space is large enough so that we don't need the
 * segmented array benefits. */
static const bool __tpie_is_not_64bit = sizeof(size_t) < sizeof(uint64_t);

///////////////////////////////////////////////////////////////////////////////
/// \brief TPIE's segmented array.
///
/// On 32-bit machines, we have problems with address space fragmentation, so
/// we allow segmented arrays that use several pieces of contiguous memory
/// masked as one.
///
/// For this reason, we do not guarantee that pointer arithmetic yields
/// anything meaningful for segmented_array elements.
///
/// Implementation note: In 64-bit address spaces, we only have a single
/// segment.
///////////////////////////////////////////////////////////////////////////////

template <typename T>
class segmented_array: public array_base<T, __tpie_is_not_64bit> {
	///////////////////////////////////////////////////////////////////////////
	/// \copydoc array_base::segmented_array(size_type, const T &)
	///////////////////////////////////////////////////////////////////////////
	segmented_array(size_type s, const T & value): array_base<T, __tpie_is_not_64bit>(s, value) {}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc array_base::segmented_array(size_type)
	///////////////////////////////////////////////////////////////////////////
	segmented_array(size_type s=0) : array_base<T, __tpie_is_not_64bit>(s) {}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc array_base::segmented_array(const array_base &)
	///////////////////////////////////////////////////////////////////////////
	segmented_array(const segmented_array & other): array_base<T, __tpie_is_not_64bit>(other) {}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Non-segmented TPIE array implementation.
///////////////////////////////////////////////////////////////////////////////

template <typename T>
class array: public array_base<T, false> {
public:
	///////////////////////////////////////////////////////////////////////////
	/// \copydoc array_base::array_base(size_type, const T &)
	///////////////////////////////////////////////////////////////////////////
	array(size_type s, const T & value): array_base<T, false>(s, value) {}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc array_base::array_base(size_type)
	///////////////////////////////////////////////////////////////////////////
	array(size_type s=0) : array_base<T, false>(s) {}

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc array_base::array_base(const array_base &)
	///////////////////////////////////////////////////////////////////////////
	array(const array & other): array_base<T, false>(other) {}

	array(array_view_base<const T> & view): array_base<T, false>(view.size()) {
		std::copy(view.begin(), view.end(), this->begin());
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a raw pointer to the array content
	///////////////////////////////////////////////////////////////////////////
	inline T * get() {return array_base<T, false>::__get();}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Return a raw pointer to the array content
	///////////////////////////////////////////////////////////////////////////
	inline const T * get() const {return array_base<T, false>::__get();}
};

template <typename T>
std::ostream & operator<<(std::ostream & o, const array<T> & a) {
	o << "[";
	bool first=true;
	for(size_t i=0; i < a.size(); ++i) {
		if (first) first = false;
		else o << ", ";
		o << a[i];
	}
	return o << "]";
}

} // namespace tpie

namespace std {

///////////////////////////////////////////////////////////////////////////////
/// \brief Enable std::swapping two tpie::arrays.
///////////////////////////////////////////////////////////////////////////////

template <typename T>
void swap(tpie::array<T> & a, tpie::array<T> & b) {
	a.swap(b);
}

} // namespace std

#endif //__TPIE_ARRAY_H__ 	
