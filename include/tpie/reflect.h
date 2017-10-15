// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, 2016, The TPIE development team
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

#ifndef __TPIE_REFLECT_H__
#define __TPIE_REFLECT_H__

#include <type_traits>
#include <iterator>
#include <array>

namespace tpie {

#define REFLECT_XSTR(s) REFLECT_STR(s)
#define REFLECT_STR(s) #s

#define REFLECT_BEGIN(name)							\
	template <typename R, typename ... T>			\
	bool reflect_impl(name *, R & r, T && ... v) {	\
		r.begin(REFLECT_XSTR(name));

#define REFLECT_VISIT(naXme)						\
	r.name(REFLECT_XSTR(naXme));					\
	if (!r.apply(v . naXme ...)) return false;

#define REFLECT_VISIT_SUPER(naXme)					\
	r.name(REFLECT_XSTR(naXme));					\
	if (!r.apply(static_cast<std::conditional_t<R::write, naXme &, const naXme &>>(v) ...)) return false;

#define REFLECT_END()							\
	r.end();									\
	return true;								\
}

template <typename R, typename T, typename ... TT>
bool reflect(R & r, T && v, TT && ... vs);

// SFINAE magic to detect if a type is trivialy serializable
template <typename T>
struct is_trivially_serializable2 {
private:
    template <typename TT>
    static char magic(TT *, typename std::enable_if<TT::is_trivially_serializable>::type *_=0);
    
    template <typename TT>
    static long magic(...);
public:
    static bool constexpr value=
		(std::is_pod<T>::value || sizeof(magic<T>((T*)nullptr))==sizeof(char)) && !std::is_pointer<T>::value;
};

// SFINAE magic to detect member functions and such
template <typename T>
class reflect_sfinae {
private:
	template <typename TT>
	static char size_and_lookup_magic(TT *,
					  decltype(std::declval<TT>().size()) * a = 0,
					  std::decay_t<decltype(std::declval<TT>()[0])> * b  = 0);
	template <typename TT>
	static long size_and_lookup_magic(...);

	template <typename TT>
	static char resize_magic(TT *,
					  decltype(std::declval<TT>().resize(0)) * a = 0);
	template <typename TT>
	static long resize_magic(...);

	template <typename TT>
	static char push_back_magic(TT *,
					  decltype(std::declval<TT &>().push_back(std::declval<std::decay_t<decltype(std::declval<TT>()[0])>>())) * a = 0);
	template <typename TT>
	static long push_back_magic(...);

	template <typename TT>
	static char data_magic(TT *,
					  decltype(std::declval<TT>().data()) * a = 0);
	template <typename TT>
	static long data_magic(...);

	template <typename TT>
	static char c_str_magic(TT *,
					  decltype(std::declval<TT>().c_str()) * a = 0);
	template <typename TT>
	static long c_str_magic(...);

	template <typename TT>
	static std::enable_if_t<is_trivially_serializable2<std::decay_t<decltype(std::declval<TT>()[0])>>::value, char> tsv_magic(TT *) {};
	
	template <typename TT>
	static long tsv_magic(...);
public:
	static constexpr bool size_and_lookup = sizeof(size_and_lookup_magic<T>((T*)nullptr)) == sizeof(char);
	static constexpr bool resize = sizeof(resize_magic<T>((T*)nullptr)) == sizeof(char);
	static constexpr bool push_back = sizeof(push_back_magic<T>((T*)nullptr)) == sizeof(char);
	static constexpr bool data = sizeof(data_magic<T>((T*)nullptr)) == sizeof(char);
	static constexpr bool c_str = sizeof(c_str_magic<T>((T*)nullptr)) == sizeof(char);
	static constexpr bool trivially_serializable_value = sizeof(tsv_magic<T>((T*)nullptr)) == sizeof(char);
};

// Use tag dispatch to reflection of known types
struct reflect_tag_impl {};
struct reflect_tag_direct {};
struct reflect_tag_trivial_array_read {};
struct reflect_tag_array_read {};
struct reflect_tag_trivial_array_write {};
struct reflect_tag_push_back_array_write {};
struct reflect_tag_array_write {};
  
template <typename D, typename R, typename ... T>
bool reflect_dispatch(reflect_tag_impl, R & r, T && ... v) {
    return reflect_impl((D*)nullptr, r, v...);
}

template <typename D, typename R, typename ... T>
bool reflect_dispatch(reflect_tag_direct, R & r, T && ... v) {
    return r(v...);
}

template <typename D, typename R, typename T, typename ... TT>
bool reflect_dispatch(reflect_tag_array_read, R & r, T && v, TT && ... vs) {
    const auto d = v.size();
    r.beginArray(d, vs.size()...);
    for (size_t i=0; i < d; ++i) {
		if (!reflect(r, v[i], vs[i]...)) return false;
    }
    r.endArray();
    return true;
}

template <typename D, typename R, typename T>
bool reflect_dispatch(reflect_tag_trivial_array_read, R & r, T && v) {
	const auto d = v.size();
    r.beginArray(d);
	if (!r(v.data(), v.size())) return false;
	r.endArray();
	return true;
}

template <typename D, typename R, typename T>
bool reflect_dispatch(reflect_tag_array_write, R & r, T && v) {
    size_t d = r.beginArray();
    v.resize(d);
    for (size_t i=0; i < d; ++i) {
		if (!r.apply(v[i])) return false;
    }
    r.endArray();
    return true;
}

template <typename D, typename R, typename T>
bool reflect_dispatch(reflect_tag_trivial_array_write, R & r, T && v) {
	size_t d = r.beginArray();
	v.resize(d);
	if (!r(&v[0], d)) return false;
	r.endArray();
	return true;
}

template <typename D, typename R, typename T>
bool reflect_dispatch(reflect_tag_push_back_array_write, R & r, T && v) {
	//static_assert(false, "Not implemented");
	return true;
}

template <size_t C, typename R, typename T>
bool reflect_static_array_dispatch(std::true_type, R & r, T && v) {
	r.beginStaticArray(C);
	if (!r(&v[0], C)) return false;
	r.endStaticArray();
	return true;
}

template <size_t C, typename R, typename T, typename ... TT>
bool reflect_static_array_dispatch(std::false_type, R & r, T && v, TT && ... vv) {
	r.beginStaticArray(C);
	for (size_t i=0; i < C; ++i)
		if (!r.applyt(v[i], vv[i]...)) return false;
	r.endStaticArray();
	return true;
}

template <bool direct, bool trivial_array_read, bool array_read, bool trivial_array_write, bool push_back_array_write, bool array_write>
struct reflect_tag_compute {
    typedef reflect_tag_impl type;
};

template <bool trivial_array_read, bool array_read, bool trivial_array_write, bool push_back_array_write, bool array_write>
struct reflect_tag_compute<true,  trivial_array_read, array_read, trivial_array_write, push_back_array_write, array_write> {
    typedef reflect_tag_direct type;
};

template <bool array_read>
struct reflect_tag_compute<false, true, array_read, false, false, false> {
    typedef reflect_tag_trivial_array_read type;
};

template <>
struct reflect_tag_compute<false, false, true, false, false, false> {
    typedef reflect_tag_array_read type;
};

template <bool push_back_array_write, bool array_write>
struct reflect_tag_compute<false, false, false, true, push_back_array_write, array_write> {
    typedef reflect_tag_trivial_array_write type;
};

template <bool array_write>
struct reflect_tag_compute<false, false, false, false, true, array_write> {
    typedef reflect_tag_push_back_array_write type;
};

template <>
struct reflect_tag_compute<false, false, false, false, false, true> {
    typedef reflect_tag_array_write type;
};

// Reflect entry point
// Reflect a bunch of T s using R
template <typename R, typename T, typename ... TT>
bool reflect(R & r, T && v, TT && ... vs) {
    typedef std::decay_t<T> D;
	typedef reflect_sfinae<D> S;
	constexpr bool dc = std::is_default_constructible<D>::value;
	constexpr bool write = R::write;
    typedef std::decay_t<T> D;
    typedef typename reflect_tag_compute<
		// direct
		(R::string && S::size_and_lookup && S::c_str && S::data) ||
		(R::trivialSerializable && is_trivially_serializable2<D>::value) ||
		(R::arithmetic && std::is_arithmetic<D>::value),
		// trivial array read
		R::trivialSerializable && S::size_and_lookup && S::data && !write && S::trivially_serializable_value,
		// array read
		S::size_and_lookup && !write,
		// trivial array write
		R::trivialSerializable && S::size_and_lookup && S::resize && S::data && write && S::trivially_serializable_value && dc,
		// push_back array write
		false, //S::size_and_lookup && S::push_back && S::resize && write,
		// array_write
		S::size_and_lookup && dc && S::resize && write
		>::type tag;
	return reflect_dispatch<D>(tag(), r, v, vs...);
}

// reflect special case for static arrays
template <typename R, typename T, std::size_t C, typename ... TT>
bool reflect(R & r, T(& v)[C], TT && ... vs) {
	typedef std::conditional_t<is_trivially_serializable2<T>::value, std::true_type, std::false_type> tag;
	return reflect_static_array_dispatch<C>(tag(), r, v, vs...);
}

// reflect special case for std::array
template <typename R, typename T, std::size_t C, typename ... TT>
bool reflect(R & r, const std::array<T, C> & v, TT && ... vs) {
	typedef std::conditional_t<is_trivially_serializable2<T>::value, std::true_type, std::false_type> tag;
	return reflect_static_array_dispatch<C>(tag(), r, v, vs...);
}

// reflect special case for std::array
template <typename R, typename T, std::size_t C, typename ... TT>
bool reflect(R & r, std::array<T, C> & v, TT && ... vs) {
	typedef std::conditional_t<is_trivially_serializable<T>::value, std::true_type, std::false_type> tag;
	return reflect_static_array_dispatch<C>(tag(), r, v, vs...);
}
} //namespace tpie

#endif //_TPIE_REFLECT_H__
