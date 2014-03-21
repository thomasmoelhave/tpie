// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014, The TPIE development team
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

#ifndef TPIE_REF_PTR_H
#define TPIE_REF_PTR_H

#include <cstddef>
#include <boost/type_traits/is_polymorphic.hpp>
#include <boost/type_traits/is_virtual_base_of.hpp>
#include <boost/type_traits/is_base_of.hpp>

namespace tpie {

/**
 * \brief Base class used for reference counting
 */
template <typename child_t>
class crtp_ref_cnt {
public:
	inline crtp_ref_cnt(): m_ref(0) {}
	inline void inc_ref() {++m_ref;}
	inline crtp_ref_cnt(const crtp_ref_cnt &): m_ref(0) {}

	inline void dec_ref() {
		--m_ref;
		if (m_ref == 0) static_cast<child_t*>(this)->ref_delete();
	}

	inline void ref_delete() {
		tpie_delete(static_cast<child_t*>(this));
	}
private:
	size_t m_ref;
};

/**
 * \brief Base class used for reference counting
 */
class ref_cnt {
public:
	inline ref_cnt(): m_ref(0) {}
	inline void inc_ref() {++m_ref;}
	inline ref_cnt(const ref_cnt &): m_ref(0) {}

	inline void dec_ref() {
		--m_ref;
		if (m_ref == 0) ref_delete();
	}

	virtual void ref_delete() {
		tpie_delete(this);
	}

	virtual ~ref_cnt() {}
private:
	size_t m_ref;
};

/**
 * \brief Pointer container for refcounted elements
 */
template <typename T>
class ref_ptr {
public:
	typedef T element_type;

	explicit inline ref_ptr(T * p=0): m_ptr(0) {
		reset(p);
	}

	inline ref_ptr(const ref_ptr & p): m_ptr(0) {
		reset(p.get());
	}

	template <typename Y>
	inline ref_ptr(const ref_ptr<Y> & p): m_ptr(0) {
		reset(p.get());
	}

	inline ref_ptr & operator=(const ref_ptr & p) {
		reset(p.get());
		return *this;
	}

	template <typename Y>
	inline ref_ptr & operator=(const ref_ptr<Y> & p) {
		reset(p.get());
		return *this;
	}

	inline ~ref_ptr() {reset(0);}

	inline T & operator*() const {return *m_ptr;}

	inline T * operator->() const {return m_ptr;}

	inline T * get() const {return m_ptr;}

	inline void reset(T * p=0) {
		if (p) p->inc_ref();
		if (m_ptr) m_ptr->dec_ref();
		m_ptr = p;
	}

	bool operator <(const ref_ptr<T> & o) const {
		return m_ptr < o.m_ptr;
	}

	bool operator ==(const ref_ptr<T> & o) const {
		return m_ptr == o.m_ptr;
	}

	bool operator!() const {return !m_ptr;}

private:
	T * m_ptr;
};

template <typename T>
inline ref_ptr<T> mkref(T * p) {return ref_ptr<T>(p);}

template <typename T>
inline ref_ptr<T> mkstackref(T & p) {p.inc_ref(); return ref_ptr<T>(&p); }

template <typename D, typename T>
void serialize(D & dst, const ref_ptr<T> & v) {
	using tpie::serialize;
	serialize(dst, *v);
}

template <typename S, typename T>
void unserialize(S & src, ref_ptr<T> & v) {
	using tpie::unserialize;
	T * x = tpie_new<T>();
	unserialize(src, *x);
	v.reset(x);
}

} //namespace tpie

#endif // TPIE_REF_PTR_H
