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
/// \file tpie/memory.h Memory management subsystem.
///////////////////////////////////////////////////////////////////////////

#ifndef __TPIE_MEMORY_H__
#define __TPIE_MEMORY_H__

#include <tpie/tpie_export.h>
#include <tpie/config.h>
#include <tpie/util.h>
#include <tpie/resource_manager.h>
#include <mutex>
#include <unordered_map>
#include <type_traits>
#include <utility>
#include <memory>
#include <atomic>
#include <typeindex>

namespace tpie {

struct memory_digest_item {
	std::string name;
	size_t count;
	size_t bytes;
	std::type_info * ti;
};

struct type_allocations {
	std::atomic_size_t bytes;
	std::atomic_size_t count;
	type_allocations() noexcept: bytes(0), count(0) {}
	type_allocations(const type_allocations & o) noexcept {bytes = (size_t)o.bytes; count = (size_t)o.count;}
	type_allocations(type_allocations && o) noexcept {bytes = (size_t)o.bytes; count = (size_t)o.count;}
	type_allocations & operator=(const type_allocations & o) noexcept {bytes = (size_t)o.bytes; count = (size_t)o.count; return *this;}
	type_allocations & operator=(type_allocations && o) noexcept {bytes = (size_t)o.bytes; count = (size_t)o.count; return *this;}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Memory management object used to track memory usage.
///////////////////////////////////////////////////////////////////////////////
class TPIE_EXPORT memory_manager final : public resource_manager {
public:
	///////////////////////////////////////////////////////////////////////////
	/// \internal
	/// Construct the memory manager object.
	///////////////////////////////////////////////////////////////////////////
	memory_manager();	

	///////////////////////////////////////////////////////////////////////////
	/// \internal
	/// Allocate the largest consecutive memory possible.
	///////////////////////////////////////////////////////////////////////////
	std::pair<uint8_t *, size_t> __allocate_consecutive(size_t upper_bound, size_t granularity);

	void register_typed_allocation(size_t bytes, const std::type_info & t);
	void register_typed_deallocation(size_t bytes, const std::type_info & t);
	
	void register_allocation(size_t bytes, const std::type_info & t) {
		register_increased_usage(bytes);
#ifndef TPIE_NDEBUG
		register_typed_allocation(bytes, t);
#else
		unused(t);
#endif
	}

	void register_deallocation(size_t bytes, const std::type_info & t) {
		register_decreased_usage(bytes);
#ifndef TPIE_NDEBUG
		register_typed_deallocation(bytes, t);
#else
		unused(t);
#endif
	}

	std::string amount_with_unit(size_t amount) const override {
		return pretty_print_size(amount);
	}

	void complain_about_unfreed_memory();
	std::unordered_map<std::type_index, memory_digest_item> memory_digest();
protected:
	void throw_out_of_resource_error(const std::string & s) override;

	std::atomic_size_t m_mutex;
	std::unordered_map<std::type_index, type_allocations> m_allocations;
};

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_init to initialize the memory manager.
///////////////////////////////////////////////////////////////////////////////
void init_memory_manager();

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_finish to deinitialize the memory manager.
///////////////////////////////////////////////////////////////////////////////
void finish_memory_manager();

///////////////////////////////////////////////////////////////////////////////
/// \brief Return a reference to the memory manager.
/// May only be called when init_memory_manager has been called.
/// See \ref tpie_init().
///////////////////////////////////////////////////////////////////////////////
TPIE_EXPORT memory_manager & get_memory_manager();

#ifndef DOXYGEN
///////////////////////////////////////////////////////////////////////////////
/// \internal
///////////////////////////////////////////////////////////////////////////////
template <typename T, 
	bool x=std::is_polymorphic<T>::value
>
struct __object_addr {
	void * operator()(T * o) {return const_cast<void*>(static_cast<const void*>(o));}
};

///////////////////////////////////////////////////////////////////////////////
/// \internal
///////////////////////////////////////////////////////////////////////////////
template <typename T>
struct __object_addr<T, true> {
	void * operator()(T * o) {return const_cast<void*>(dynamic_cast<const void *>(o));}
};
#endif

///////////////////////////////////////////////////////////////////////////////
/// Cast between pointer types. If the input pointer is polymorphic, its base
/// address is found, and that is then casted to the output type.
///
/// \tparam D A non-polymorphic pointer type.
/// \tparam T Any type.
///////////////////////////////////////////////////////////////////////////////
template <typename D, typename T>
inline D ptr_cast(T * t) { return reinterpret_cast<D>(__object_addr<T>()(t)); }


template <typename T>
inline T * tpie_allocate() {
	if(!std::is_polymorphic<T>::value) return reinterpret_cast<T *>(new uint8_t[sizeof(T)]);	
	uint8_t * x = new uint8_t[sizeof(T)+sizeof(size_t)];
	*reinterpret_cast<size_t*>(x) = sizeof(T);
	return reinterpret_cast<T*>(x + sizeof(size_t));
}

template <typename T>
inline void tpie_deallocate(T * p) {
	uint8_t * pp = ptr_cast<uint8_t *>(p);
	if(!std::is_polymorphic<T>::value)
		delete[] pp;
	else if (pp != nullptr)
		delete[] (pp - sizeof(size_t));
}

template <typename T>
inline size_t tpie_size(T * p) {
	if(!std::is_polymorphic<T>::value) return sizeof(T);
	uint8_t * x = ptr_cast<uint8_t *>(p);
	return * reinterpret_cast<size_t *>(x - sizeof(size_t));
}


///////////////////////////////////////////////////////////////////////////////
/// \internal
/// Used to perform allocations in a safe manner.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
struct array_allocation_scope_magic {
	size_t size;
	T * data;
	array_allocation_scope_magic(size_t s): size(0), data(0) {
		get_memory_manager().register_allocation(s * sizeof(T), typeid(T) );
		size=s;
	}

	T * allocate() {
		data = new T[size];
		return data;
	}

	T * finalize() {
		T * d = data;
		size = 0;
		data = 0;
		return d;
	}

	~array_allocation_scope_magic() {
		if(size) get_memory_manager().register_deallocation(size*sizeof(T), typeid(T) );
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \internal
/// Used in tpie_new to perform allocations in a safe manner.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
struct allocation_scope_magic {
	size_t deregister;
	T * data;
	allocation_scope_magic(): deregister(0), data(0) {}
	
	T * allocate() {
		get_memory_manager().register_allocation(sizeof(T), typeid(T));
		deregister = sizeof(T);
		data = tpie_allocate<T>();
		return data;
	}

	T * finalize() {
		T * d = data;
		deregister = 0;
		data = 0;
		return d;
	}
	
	~allocation_scope_magic() {
		tpie_deallocate(data);
		if (deregister) get_memory_manager().register_deallocation(deregister, typeid(T));
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Allocate a new array and register its memory usage.
/// \param size The number of elements in the new array.
/// \return The new array.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline T * tpie_new_array(size_t size) {
	array_allocation_scope_magic<T> m(size);
	m.allocate();
	return m.finalize();
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Allocate an element of the type given as template parameter, and
/// register its memory usage with TPIE.
///
/// The implementation of tpie_new either uses variadic templates (if supported
/// by the compiler) or a bunch of tpie_new overloads to support a variable
/// number of constructor parameters.
///
/// \tparam T The type of element to allocate
/// \tparam Args The variadic number of types of constructor parameters.
/// \param args The variadic number of arguments to pass to the constructor of
/// T.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename ... Args>
inline T * tpie_new(Args &&... args) {
	allocation_scope_magic<T> m; 
	new(m.allocate()) T(std::forward<Args>(args)...);
	return m.finalize();
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Delete an object allocated with tpie_new.
/// \param p the object to delete
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline void tpie_delete(T * p) throw() {
	if (p == 0) return;
	get_memory_manager().register_deallocation(tpie_size(p), typeid(*p));
	p->~T();
	tpie_deallocate(p);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Delete an array allocated with tpie_new_array.
/// \param a The array to delete.
/// \param size The size of the array in elements as passed to tpie_new_array.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
inline void tpie_delete_array(T * a, size_t size) throw() {
	if (a == 0) return;
	get_memory_manager().register_deallocation(sizeof(T) * size, typeid(T));
	delete[] a;
}

struct tpie_deleter {
	template <typename T>
	void operator()(T * t) {
		tpie_delete(t);
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief like std::unique_ptr, but delete the object with tpie_delete.
/// \tparam T the type of the object.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
using unique_ptr=std::unique_ptr<T, tpie_deleter>;

///////////////////////////////////////////////////////////////////////////////
/// \brief Create a new unique object using tpie::new
/// \tparam T the type of the object.
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename ... TT>
inline unique_ptr<T> make_unique(TT && ... tt) {
	return unique_ptr<T>(tpie_new<T>(std::forward<TT>(tt)...));
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Bucket used for memory counting
///////////////////////////////////////////////////////////////////////////////
class memory_bucket {
public:
	memory_bucket(): count(0) {}
	
	std::atomic_size_t count;
	std::string name;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Class storring a reference to a memory bucket
///
/// We do not use raw pointers manly because the go into overloading resolution
/// with 0.
///////////////////////////////////////////////////////////////////////////////
class memory_bucket_ref {
public:
	explicit memory_bucket_ref(memory_bucket * b=nullptr) noexcept: bucket(b) {}
	explicit operator bool() const noexcept {return bucket != nullptr;}
	memory_bucket_ref(const memory_bucket_ref & o)  = default;
	memory_bucket_ref(memory_bucket_ref && o) = default;
	memory_bucket_ref & operator=(const memory_bucket_ref & o) = default;
	memory_bucket_ref & operator=(memory_bucket_ref && o) = default;
	memory_bucket & operator*() noexcept {return *bucket;} 
	const memory_bucket & operator*() const noexcept {return *bucket;} 
	memory_bucket * operator->() noexcept {return bucket;} 
	const memory_bucket * operator->() const noexcept {return bucket;}
	friend bool operator ==(const memory_bucket_ref & l, const memory_bucket_ref & r) noexcept {return l.bucket == r.bucket;}
	friend bool operator !=(const memory_bucket_ref & l, const memory_bucket_ref & r) noexcept {return l.bucket != r.bucket;}
private:
	memory_bucket * bucket;
};

///////////////////////////////////////////////////////////////////////////////
/// \brief A allocator object usable in STL containers, using the TPIE
/// memory manager.
/// \tparam T The type of the elements that can be allocated.
///////////////////////////////////////////////////////////////////////////////
template <class T>
class allocator {
private:
    typedef std::allocator<T> a_t;
    typedef std::allocator_traits<a_t> at_t;
    a_t a;
public:
	memory_bucket_ref bucket;

	typedef typename at_t::size_type size_type;
	typedef typename at_t::difference_type difference_type;
	typedef typename at_t::pointer pointer;
	typedef typename at_t::const_pointer const_pointer;
	typedef typename at_t::value_type value_type;

	typedef std::true_type propagate_on_container_copy_assignment;
	typedef std::true_type propagate_on_container_move_assignment;
	typedef std::true_type propagate_on_container_swap;
	
	allocator() = default;
	allocator(memory_bucket_ref bucket) noexcept : bucket(bucket) {}
	allocator(const allocator & o) noexcept : bucket(o.bucket) {}
	template <typename T2>
	allocator(const allocator<T2> & o) noexcept : bucket(o.bucket) {}
	allocator(allocator && o) noexcept : bucket(std::move(o.bucket)) {}
	template <typename T2>
	allocator(allocator<T2> && o) noexcept : bucket(std::move(o.bucket)) {}

	allocator & operator=(const allocator & o) noexcept {bucket = o.bucket; return *this;}
	template <typename T2>
	allocator & operator=(const allocator<T2> & o) noexcept {bucket = o.bucket; return *this;}
	allocator & operator=(allocator && o) noexcept {bucket = std::move(o.bucket); return *this;}
	template <typename T2>
	allocator & operator=(allocator<T2> && o) noexcept {bucket = std::move(o.bucket); return *this;}

    template <class U> struct rebind {typedef allocator<U> other;};

    T * allocate(size_t size) {
		get_memory_manager().register_allocation(size * sizeof(T), typeid(T));
		if (bucket) bucket->count += size * sizeof(T);
		T * res = a.allocate(size);
		return res;
    }

    void deallocate(T * p, size_t n) {
		if (p == 0) return;
		if (bucket) bucket->count -= n * sizeof(T);
		get_memory_manager().register_deallocation(n * sizeof(T), typeid(T));
		return a.deallocate(p, n);
    }
    size_t max_size() const noexcept {return at_t::max_size(a);}

	template <typename U, typename ...TT>
	void construct(U * p, TT &&...x) {
		at_t::construct(a, p, std::forward<TT>(x)...);
	}


	template <typename U>
    void destroy(U * p) {
		at_t::destroy(a, p);
	}

	friend bool operator==(const allocator & l, const allocator & r) noexcept {return l.bucket == r.bucket;}
	friend bool operator!=(const allocator & l, const allocator & r) noexcept {return l.bucket != r.bucket;}

	template <typename U>
	friend class allocator;

		
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Find the largest amount of memory that can be allocated as a single
/// chunk.
///////////////////////////////////////////////////////////////////////////////
TPIE_EXPORT size_t consecutive_memory_available(size_t granularity=5*1024*1024);

} //namespace tpie

#endif //__TPIE_MEMORY_H__
