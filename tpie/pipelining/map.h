// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2011, 2012, The TPIE development team
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

#ifndef __TPIE_PIPELINING_MAP_H__
#define __TPIE_PIPELINING_MAP_H__

#include <tpie/pipelining/node.h>
#include <tpie/pipelining/pipe_base.h>
#include <tpie/pipelining/factory_helpers.h>
#include <tpie/pipelining/node_name.h>
#include <type_traits>

namespace tpie::pipelining {
namespace bits {

template <typename T>
struct unary_traits_imp;

template <typename C, typename R, typename A>
struct unary_traits_imp<R(C::*)(A)> {
	typedef A argument_type;
	typedef R return_type;
};

template <typename C, typename R, typename A>
struct unary_traits_imp<R(C::*)(A) const > {
	typedef A argument_type;
	typedef R return_type;
};

template <typename R, typename A>
struct unary_traits_imp<R(*)(A)> {
	typedef A argument_type;
	typedef R return_type;
};

template <typename T>
struct unary_traits: public unary_traits_imp<decltype(&T::operator()) > {};

template <typename dest_t, typename F>
class map_t: public node {
private:
	F functor;
	dest_t dest;
public:
	typedef typename std::decay<typename unary_traits<F>::argument_type>::type item_type;
	
	map_t(dest_t dest, const F & functor):
		functor(functor), dest(std::move(dest)) {
		set_name(bits::extract_pipe_name(typeid(F).name()), PRIORITY_NO_NAME);
	}
	
	void push(const item_type & item) {
		dest.push(functor(item));
	}
};

template <typename dest_t, typename F>
class map_temp_t: public node {
private:
	F functor;
	dest_t dest;
public:
	map_temp_t(dest_t dest, const F & functor):
		functor(functor), dest(std::move(dest)) {
		set_name(bits::extract_pipe_name(typeid(F).name()), PRIORITY_NO_NAME);
	}
	
	template <typename T>
	void push(const T & item) {
		dest.push(functor(item));
	}
};

template <typename F>
class map_sink_t: public node {
private:
	F functor;
public:
	typedef typename std::decay<typename unary_traits<F>::argument_type>::type item_type;

	map_sink_t(const F & functor):
		functor(functor) {
		set_name(bits::extract_pipe_name(typeid(F).name()), PRIORITY_NO_NAME);
	}
	
	void push(const item_type & item) {
		functor(item);
	}
};

template <typename src_t, typename F>
class pull_map_t: public node {
private:
	F functor;
	src_t src;
public:
	pull_map_t(src_t src, const F & functor):
		functor(functor), src(std::move(src)) {
		set_name(bits::extract_pipe_name(typeid(F).name()), PRIORITY_NO_NAME);
	}
	
	auto pull() {
		return functor(src.pull());
	}
	
	bool can_pull() {
		return src.can_pull();
	}
};


template <typename T>
struct has_argument_type {
	typedef char yes[1];
	typedef char no[2];

	// This does not seem to work as well as it should
	// template <typename C>
	// static yes& test(typename unary_traits_imp<decltype(&C::operator())>::argument_type *);

	template <typename C>
	static yes& test(decltype(&C::operator()) *);

	template <typename>
	static no& test(...);
	static const bool value = sizeof(test<T>(nullptr)) == sizeof(yes);
};

} //namespace bits

///////////////////////////////////////////////////////////////////////////////
/// \brief Pipelining nodes that applies to given functor to items in
/// the stream.
/// \param functor The functor that should be applied to items
///////////////////////////////////////////////////////////////////////////////
template <typename F, typename = typename std::enable_if<bits::has_argument_type<F>::value>::type>
pipe_middle<tfactory<bits::map_t, Args<F>, F> > map(const F & functor) {
	return {functor};
}

template <typename F, typename = typename std::enable_if<!bits::has_argument_type<F>::value>::type>
pipe_middle<tfactory<bits::map_temp_t, Args<F>, F> > map(const F & functor) {
	return {functor};
}

template <typename F>
pipe_end<termfactory<bits::map_sink_t<F>, F> > map_sink(const F & functor) {
	return {functor};
}

template <typename F>
pullpipe_middle<tfactory<bits::pull_map_t, Args<F>, F>> pull_map(const F & functor) {
	return {functor};
}

} //namespace terrastream::pipelining

#endif //__TPIE_PIPELINING_MAP_H__
