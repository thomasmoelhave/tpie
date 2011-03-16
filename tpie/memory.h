// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
//
// Copyright 2010, 2011 SCALGO development
#ifndef __TPIE_MEMORY_H__
#define __TPIE_MEMORY_H__

#include <utility>
#include <vector>
namespace tpie {

//#define CPP0X_VARAIADIC
//#define CPP0X_MOVABLE

class memory_manager {
public:
	size_t used() const throw();
	void register_allocation(size_t bytes);
	void register_deallocation(size_t bytes);
	size_t consecutive_memory_available();
};

memory_manager & get_memory_manager();

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

#ifdef CPP0X_VARAIADIC
template <typename T,typename... TT>
inline T * tpie_new(TT... vals) {
	return allocation_scope_magic(sizeof(T))(new T(vals...));
}
#else //CPP0X_VARAIADIC
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
#endif //CPP0X_VARAIADIC

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
#ifdef CPP0X_MOVABLE
	inline auto_ptr(auto_ptr && o): elm(0) {reset(o.release());}
#endif //CPP0X_MOVABLE
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

#ifdef CPP0X_VARAIADIC
	template <typename ...TT>
	inline void construct(T * p, TT &&...x) {a.construct(p, x...);}
#elif defined(CPP0X_MOVABLE)
	template <typename TT>
	inline void construct(T * p, TT && val) {a.construct(p, val);}
#endif
    inline void construct(T * p, const T& val) {a.construct(p, val);}
    inline void destroy(T * p) {a.destroy(p);}    
};
} //tpie

namespace std {
template <typename T>
void swap(tpie::auto_ptr<T> & a, tpie::auto_ptr<T> & b) {
	T * t =a.release();
	a.reset(b.release());
	b.reset(t);
}
} //namespace std

#endif //__TPIE_MEMORY_H__
