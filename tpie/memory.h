// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
//
// Copyright 2011, The TPIE development team
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

#ifndef __TPIE_MEMORY_H__
#define __TPIE_MEMORY_H__

#include <tpie/config.h>
#include <boost/thread/mutex.hpp>

#include <utility>

namespace tpie {

struct out_of_memory_error : public std::bad_alloc {
	const char * msg;
	out_of_memory_error(const char * s) : msg(s) { }
	virtual const char* what() const throw() {return msg;}
};


/**
 * \brief Memory managment object used to track memory usage.
 */
class memory_manager {
public:
	enum enforce_t {
		ENFORCE_IGNORE,
		ENFORCE_WARN,
		ENFORCE_THROW
	};

	memory_manager();

	inline size_t used() const throw() {return m_used;}
	size_t available() const throw();

	inline size_t limit() const throw() {return m_limit;}
	void set_limit(size_t new_limit);

	void register_deallocation(size_t bytes);
	void register_allocation(size_t bytes);

	void set_enforcement(enforce_t e);
	inline enforce_t enforcement() {return m_enforce;}
	
	std::pair<uint8_t *, size_t> __allocate_consecutive(size_t upper_bound, size_t granularity);
private:
	size_t m_used;
	size_t m_limit;
	size_t m_maxExceeded;
	enforce_t m_enforce;
	boost::mutex m_mutex;
};

void init_memory_manager();
void finish_memory_manager();
memory_manager & get_memory_manager();

/**
 * \internal
 * Used to preform allocations in a safe manner
 */
struct allocation_scope_magic {
	size_t deregister;
	inline allocation_scope_magic(size_t size) {
		get_memory_manager().register_allocation(size);
		deregister = size;
	}

	template <typename T>
	inline T operator()(T x) {deregister = 0; return x;}

	inline ~allocation_scope_magic() {if(deregister) get_memory_manager().register_deallocation(deregister);}
};

template <typename T>
inline T * tpie_new_array(size_t size) {
	return allocation_scope_magic(sizeof(T)*size)(new T[size]);
}

#ifdef TPIE_CPP_VARIADIC_TEMPLATES
template <typename T,typename... TT>
inline T * tpie_new(TT... vals) {
	return allocation_scope_magic(sizeof(T))(new T(vals...));
}
#else //TPIE_CPP_VARIADIC_TEMPLATES
template <typename T>
inline T * tpie_new() {
	return allocation_scope_magic(sizeof(T))(new T);
}

template <typename T, typename T1>
inline T * tpie_new(T1 t1) {
	return allocation_scope_magic(sizeof(T))(new T(t1));
}

template <typename T, typename T1, typename T2>
inline T * tpie_new(T1 t1, T2 t2) {
	return allocation_scope_magic(sizeof(T))(new T(t1, t2));
}

template <typename T, typename T1, typename T2, typename T3>
inline T * tpie_new(T1 t1, T2 t2, T3 t3) {
	return allocation_scope_magic(sizeof(T))(new T(t1, t2, t3));
}

template <typename T, typename T1, typename T2, typename T3, typename T4>
inline T * tpie_new(T1 t1, T2 t2, T3 t3, T4 t4) {
	return allocation_scope_magic(sizeof(T))(new T(t1, t2, t3, t4));
}

template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
inline T * tpie_new(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) {
	return allocation_scope_magic(sizeof(T))(new T(t1, t2, t3, t4, t5));
}
#endif //TPIE_CPP_VARIADIC_TEMPLATES

template <typename T>
inline void tpie_delete(T * p) throw() {
	if (p == 0) return;
	get_memory_manager().register_deallocation(sizeof(T));
	delete p;
}

template <typename T>
inline void tpie_delete_array(T * a, size_t size) throw() {
	if (a == 0) return;
	get_memory_manager().register_deallocation(sizeof(T) * size);
	delete[] a;
}

template <typename T>
class auto_ptr {
private:
	T * elm;
public:
	inline T * release() throw () {T * t=elm; elm=0; return t;}
	inline T * get() const throw() {return elm;}
	inline T & operator*() const throw() {return *elm;}
	inline T * operator->() const throw() {return elm;}
	inline void reset(T * o=0) throw () {
		if (elm) tpie_delete<T>(elm);
		elm=o;
	}
	inline auto_ptr(T * o=0): elm(0) {reset(o);}
	inline auto_ptr(auto_ptr & o): elm(0) {reset(o.release());}
#ifdef TPIE_CPP_RVALUE_REFERENCE
	inline auto_ptr(auto_ptr && o): elm(0) {reset(o.release());}
#endif //TPIE_CPP_RVALUE_REFERENCE
	inline ~auto_ptr() throw() {reset();}
};


template <class T>
class allocator {
private:
    typedef std::allocator<T> a_t;
    a_t a;
public:
    typedef typename a_t::size_type size_type;
    typedef typename a_t::difference_type difference_type;
	typedef typename a_t::pointer pointer;
	typedef typename a_t::const_pointer const_pointer;
	typedef typename a_t::reference reference;
	typedef typename a_t::const_reference const_reference;
    typedef typename a_t::value_type value_type;

    template <class U> struct rebind {typedef allocator<U> other;};

    inline T * allocate(size_t size, const void * hint=0) {
		get_memory_manager().register_allocation(size * sizeof(T));
		return a.allocate(size, hint);
    }

    inline void deallocate(T * p, size_t n) {
		get_memory_manager().register_deallocation(n * sizeof(T));
		return a.deallocate(p, n);
    }
    inline size_t max_size() const {return a.max_size();}

#ifdef TPIE_CPP_RVALUE_REFERENCE
#ifdef TPIE_CPP_VARIADIC_TEMPLATES
	template <typename ...TT>
	inline void construct(T * p, TT &&...x) {a.construct(p, x...);}
#else
	template <typename TT>
	inline void construct(T * p, TT && val) {a.construct(p, val);}
#endif
#endif
    inline void construct(T * p, const T& val) {a.construct(p, val);}
    inline void destroy(T * p) {a.destroy(p);}    
};


size_t consecutive_memory_available(size_t granularity=5*1024*1024);

} //namespace tpie

namespace std {
template <typename T>
void swap(tpie::auto_ptr<T> & a, tpie::auto_ptr<T> & b) {
	T * t =a.release();
	a.reset(b.release());
	b.reset(t);
}
} //namespace std

#endif //__TPIE_MEMORY_H__
