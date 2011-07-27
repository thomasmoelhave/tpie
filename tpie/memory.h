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

///////////////////////////////////////////////////////////////////////////
/// \file tpie/memory.h Declares tpie memory managment funcionality
///////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_MEMORY_H__
#define __TPIE_MEMORY_H__

#include <tpie/config.h>
#include <tpie/util.h>
#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>
#include <boost/type_traits/is_polymorphic.hpp>
#include <utility>

namespace tpie {

////////////////////////////////////////////////////////////////////////////////
/// \brief Thrown when trying to allocate to much memory
///
/// Throw by the tpie allocator when the memory limit is exceeded during an
/// allocation assuming the memory limit enforcment policy is set to THROW
////////////////////////////////////////////////////////////////////////////////
struct out_of_memory_error : public std::bad_alloc {
	const char * msg;
	out_of_memory_error(const char * s) : msg(s) { }
	virtual const char* what() const throw() {return msg;}
};


////////////////////////////////////////////////////////////////////////////////
/// \brief Memory managment object used to track memory usage.
////////////////////////////////////////////////////////////////////////////////
class memory_manager {
public:
	/// Memory limit enforcement policies.
	enum enforce_t {
		/// Ignore when running out of memory
		ENFORCE_IGNORE,
		/// \brief Log a warning when the memory limit is exceeded
		///
		/// Note that not all violations will be logged
		ENFORCE_WARN,
		/// Throw a out_of_memory_error when the memory limit is exceede
		ENFORCE_THROW
	};

	/// Return the current amount of memory used
	inline size_t used() const throw() {return m_used;}
   
	/// Return the amout of memory still availabe to allocation
	size_t available() const throw();

	/// Return the memory limit
	inline size_t limit() const throw() {return m_limit;}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief Update the memory limit
	/// If the memory limit becommes exceede by decreesing the limit,
	/// no exception will be thrown
	/// \param new_limit The new memory limit in bytes
	///////////////////////////////////////////////////////////////////////////
	void set_limit(size_t new_limit);

	///////////////////////////////////////////////////////////////////////////
	/// \brief Set how the memory limit should be enforced
	/// \param e The new enforcement policy
	///////////////////////////////////////////////////////////////////////////
	void set_enforcement(enforce_t e);

	///////////////////////////////////////////////////////////////////////////
	/// \brief return the current memory limit enforcement policy
	///////////////////////////////////////////////////////////////////////////
	inline enforce_t enforcement() {return m_enforce;}

	///////////////////////////////////////////////////////////////////////////
	/// \internal
	/// Register that more memory has been used, 
	/// possible throw a warning or an exception if the memory limit
	/// becomes exceeded depending on the enforcement
	///////////////////////////////////////////////////////////////////////////
	void register_allocation(size_t bytes);

	///////////////////////////////////////////////////////////////////////////
	/// \internal
	/// Register that some memory has been freed
	///////////////////////////////////////////////////////////////////////////
	void register_deallocation(size_t bytes);

	///////////////////////////////////////////////////////////////////////////
	/// \internal
	/// Construct the memory manager object
	///////////////////////////////////////////////////////////////////////////
	memory_manager();	

	///////////////////////////////////////////////////////////////////////////
	/// \internal
	/// Allocate the largetst consecutive memory possible
	///////////////////////////////////////////////////////////////////////////
	std::pair<uint8_t *, size_t> __allocate_consecutive(size_t upper_bound, size_t granularity);

#ifndef TPIE_NDEBUG
	void __register_pointer(void * p, size_t size, const std::type_info & t);
	void __unregister_pointer(void * p, size_t size, const std::type_info & t);
	void __complain_about_unfreed_memory();
#endif

private:
	size_t m_used;
	size_t m_limit;
	size_t m_maxExceeded;
	enforce_t m_enforce;
	boost::mutex m_mutex;
#ifndef TPIE_NDEBUG
	boost::unordered_map<void *, std::pair<size_t, const std::type_info *> > m_pointers;
#endif
};

///////////////////////////////////////////////////////////////////////////
/// \internal
/// Initialize the memory manager, this should not be called directly
/// instead functions from tpie.h should be called.
///////////////////////////////////////////////////////////////////////////
void init_memory_manager();

///////////////////////////////////////////////////////////////////////////
/// \internal
/// Finish up the memory manager, this should not be called directly
/// instead functions from tpie.h should be called
///////////////////////////////////////////////////////////////////////////
void finish_memory_manager();

///////////////////////////////////////////////////////////////////////////
/// \brief Return a reference to the memory manager
/// Note may only be called when init_memory_manager has been called
///////////////////////////////////////////////////////////////////////////
memory_manager & get_memory_manager();

///////////////////////////////////////////////////////////////////////////
/// \internal
/// Register a pointer for debugging memory leeks and such
///////////////////////////////////////////////////////////////////////////
inline void __register_pointer(void * p, size_t size, const std::type_info & t) {
#ifndef TPIE_NDEBUG
	get_memory_manager().__register_pointer(p, size, t);
#else
	unused(p);
	unused(size);
	unused(t);
#endif
}

///////////////////////////////////////////////////////////////////////////
/// \internal
/// Unregister a registered pointer
///////////////////////////////////////////////////////////////////////////
inline void __unregister_pointer(void * p, size_t size, const std::type_info & t) {
#ifndef TPIE_NDEBUG
	get_memory_manager().__unregister_pointer(p, size, t);
#else
	unused(p);
	unused(size);
	unused(t);
#endif
}

///////////////////////////////////////////////////////////////////////////
/// \internal
///////////////////////////////////////////////////////////////////////////
template <typename T, bool x=boost::is_polymorphic<T>::value >
struct __object_addr {
	inline void * operator()(T * o) {return static_cast<void *>(o);}
};

///////////////////////////////////////////////////////////////////////////
/// \internal
///////////////////////////////////////////////////////////////////////////
template <typename T>
struct __object_addr<T, true> {
	inline void * operator()(T * o) {return dynamic_cast<void *>(o);}
};

///////////////////////////////////////////////////////////////////////////
/// Cast between pointer types, if the input pointer is polymorpic its base address
/// is found and that is then casted to the output type.
///
/// \tparam D must be a none polymorphic pointer type
/// \tparam T any type
///////////////////////////////////////////////////////////////////////////
template <typename D, typename T>
inline D ptr_cast(T * t) { return reinterpret_cast<D>(__object_addr<T>()(t)); }


template <typename T>
inline T * __allocate() {
	if(!boost::is_polymorphic<T>::value) return reinterpret_cast<T *>(new uint8_t[sizeof(T)]);	
	uint8_t * x = new uint8_t[sizeof(T)+sizeof(size_t)];
	*reinterpret_cast<size_t*>(x) = sizeof(T);
	return reinterpret_cast<T*>(x + sizeof(size_t));
}

template <typename T>
inline void __deallocate(T * p) {
	if(!boost::is_polymorphic<T>::value) 
		return delete[] reinterpret_cast<uint8_t*>(p);
	else
		return delete[] (ptr_cast<uint8_t *>(p) - sizeof(size_t));
}

template <typename T>
inline size_t tpie_size(T * p) {
	if(!boost::is_polymorphic<T>::value) return sizeof(T);
	uint8_t * x = ptr_cast<uint8_t *>(p);
	return * reinterpret_cast<size_t *>(x - sizeof(size_t));
}


///////////////////////////////////////////////////////////////////////////
/// \internal
/// Used to preform allocations in a safe manner
///////////////////////////////////////////////////////////////////////////
template <typename T>
struct array_allocation_scope_magic {
	size_t size;
	T * data;
	inline array_allocation_scope_magic(size_t s): size(0), data(0) {
		get_memory_manager().register_allocation(s * sizeof(T) );
		size=s;
	}

	inline T * allocate() {
		data = new T[size];
		__register_pointer(data, size*sizeof(T), typeid(T));
		return data;
	}

	T * finalize() {
		T * d = data;
		size = 0;
		data = 0;
		return d;
	}

	inline ~array_allocation_scope_magic() {
		if(size) get_memory_manager().register_deallocation(size*sizeof(T));
	}
};

///////////////////////////////////////////////////////////////////////////
/// \internal
/// Used to preform allocations in a safe manner
///////////////////////////////////////////////////////////////////////////
template <typename T>
struct allocation_scope_magic {
	size_t deregister;
	T * data;
	inline allocation_scope_magic(): deregister(0), data(0) {}
	
	inline T * allocate() {
		get_memory_manager().register_allocation(sizeof(T));
		deregister = sizeof(T);
		data = __allocate<T>();
		__register_pointer(data, sizeof(T), typeid(T));
		return data;
	}

	T * finalize() {
		T * d = data;
		deregister = 0;
		data = 0;
		return d;
	}
	
	inline ~allocation_scope_magic() {
		if (data) __unregister_pointer(data, sizeof(T), typeid(T));
		delete[] reinterpret_cast<uint8_t*>(data);
		if (deregister) get_memory_manager().register_deallocation(deregister);
	}
};

///////////////////////////////////////////////////////////////////////////
/// \brief Allocate a new array, and register its memory usage
/// \param size The number of elements in the new array
/// \return The new array
///////////////////////////////////////////////////////////////////////////
template <typename T>
inline T * tpie_new_array(size_t size) {
	array_allocation_scope_magic<T> m(size);
	m.allocate();
	for(size_t i=0; i < size; ++i)
		new(m.data + i) T();
	return m.finalize();
}

#ifdef TPIE_CPP_VARIADIC_TEMPLATES
///////////////////////////////////////////////////////////////////////////
/// \brief Like new but also register the memory usage
///////////////////////////////////////////////////////////////////////////
template <typename T,typename... TT>
inline T * tpie_new(TT... vals) {
	allocation_scope_magic<T> m;
	new(m.allocate()) T(vals...);
	return m.finalize();
}
#else //TPIE_CPP_VARIADIC_TEMPLATES

///////////////////////////////////////////////////////////////////////////
/// \brief Like new but also register the memory usage
/// \tparam T the tpie of of the object to allocate
/// \return The allocated object
///////////////////////////////////////////////////////////////////////////
template <typename T>
inline T * tpie_new() {
	allocation_scope_magic<T> m;
	new(m.allocate()) T();
	return m.finalize();
}

///////////////////////////////////////////////////////////////////////////
/// \brief Like new but also register the memory usage
/// \tparam T the tpie of of the object to allocate
/// \param t1 The first argument to the constructor
/// \return The allocated object
///////////////////////////////////////////////////////////////////////////
template <typename T, typename T1>
inline T * tpie_new(T1 t1) {
	allocation_scope_magic<T> m;
	new(m.allocate()) T(t1);
	return m.finalize();
}

///////////////////////////////////////////////////////////////////////////
/// \brief Like new but also register the memory usage
/// \tparam T the tpie of of the object to allocate
/// \param t1 The first argument to the constructor
/// \param t2 The second argument to the constructor
/// \return The allocated object
///////////////////////////////////////////////////////////////////////////
template <typename T, typename T1, typename T2>
inline T * tpie_new(T1 t1, T2 t2) {
	allocation_scope_magic<T> m;
	new(m.allocate()) T(t1, t2);
	return m.finalize();
}

template <typename T, typename T1, typename T2, typename T3>
inline T * tpie_new(T1 t1, T2 t2, T3 t3) {
	allocation_scope_magic<T> m;
	new(m.allocate()) T(t1, t2, t3);
	return m.finalize();
}

template <typename T, typename T1, typename T2, typename T3, typename T4>
inline T * tpie_new(T1 t1, T2 t2, T3 t3, T4 t4) {
	allocation_scope_magic<T> m;
	new(m.allocate()) T(t1, t2, t3, t4);
	return m.finalize();
}

template <typename T, typename T1, typename T2, typename T3, typename T4, typename T5>
inline T * tpie_new(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) {
	allocation_scope_magic<T> m;
	new(m.allocate()) T(t1, t2, t3, t4, t5);
	return m.finalize();
}
#endif //TPIE_CPP_VARIADIC_TEMPLATES

///////////////////////////////////////////////////////////////////////////
/// \brief Delete an object allocated with tpie_new
/// \param p the object to delet
///////////////////////////////////////////////////////////////////////////
template <typename T>
inline void tpie_delete(T * p) throw() {
	if (p == 0) return;
	get_memory_manager().register_deallocation(tpie_size(p));
	__unregister_pointer(ptr_cast<void *>(p), tpie_size(p), typeid(*p));
	p->~T();
	__deallocate(p);
}

///////////////////////////////////////////////////////////////////////////
/// \brief Delete an array allocated with tpie_new_array
/// \param a The array to delete
/// \param size The size of the array in elements as passed to tpie_new_array
///////////////////////////////////////////////////////////////////////////
template <typename T>
inline void tpie_delete_array(T * a, size_t size) throw() {
	if (a == 0) return;
	get_memory_manager().register_deallocation(sizeof(T) * size);
	__unregister_pointer(a, sizeof(T) * size, typeid(T) );
	delete[] a;
}

template <typename T>
struct auto_ptr_ref {
	T * m_ptr;
	explicit inline auto_ptr_ref(T * ptr) throw(): m_ptr(ptr) {};
};

///////////////////////////////////////////////////////////////////////////
/// \brief like std::auto_ptr, but delete the object with tpie_delete
/// \tparam T the type of the object
///////////////////////////////////////////////////////////////////////////
template <typename T>
class auto_ptr {
private:
	T * elm;
	// auto_ptr(const auto_ptr & o) {unused(o); assert(false);}
	// auto_ptr & operator = (const auto_ptr & o) {unused(o); assert(false);}
public:
	typedef T element_type;

	// D.10.1.1 construct/copy/destroy:
	explicit inline auto_ptr(T * o=0) throw(): elm(0){reset(o);}	
	inline auto_ptr(auto_ptr & o) throw(): elm(0) {reset(o.release());}
	template <class Y> 
	inline auto_ptr(auto_ptr<Y> & o) throw(): elm(0) {reset(o.release());}
	
	inline auto_ptr & operator =(auto_ptr & o) throw() {reset(o.release());}
	template <class Y>
	inline auto_ptr & operator =(auto_ptr<Y> & o) throw() {reset(o.release());}
	inline auto_ptr & operator =(auto_ptr_ref<T> r) throw() {reset(r.m_ptr);}

	inline ~auto_ptr() throw() {reset();}

	// D.10.1.2 members:
	inline T & operator*() const throw() {return *elm;}
	inline T * operator->() const throw() {return elm;}
	inline T * get() const throw() {return elm;}	
	inline T * release() throw () {T * t=elm; elm=0; return t;}	
	inline void reset(T * o=0) throw () {
		if (o == elm) return;
		tpie_delete<T>(elm); 
		elm=o; 
	}

	// D.10.1.3 conversions:
	inline auto_ptr(auto_ptr_ref<T> r) throw(): elm(0)  {reset(r.m_ptr);}
	
	template <class Y> 
		inline operator auto_ptr_ref<Y>() throw() {return auto_ptr_ref<Y>(release());}

	template<class Y> 
		inline operator auto_ptr<Y>() throw() {return auto_ptr<Y>(release());}
};

///////////////////////////////////////////////////////////////////////////
/// \brief A allocator object usable in stl containers, using the tpie
/// memory manager
/// \tparam T The type of the elements that can be allocated
///////////////////////////////////////////////////////////////////////////
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
		T * res = a.allocate(size, hint);
		__register_pointer(res, size, typeid(T));
		return res;
    }

    inline void deallocate(T * p, size_t n) {
		if (p == 0) return;
		__unregister_pointer(p, n, typeid(T));
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


///////////////////////////////////////////////////////////////////////////
/// \brief Calculate the largest amount of memory that can be 
/// allocated as a single chunk
///////////////////////////////////////////////////////////////////////////
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
