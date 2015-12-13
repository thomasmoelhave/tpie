// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014 The TPIE development team
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

#ifndef _TPIE_BTREE_BASE_H_
#define _TPIE_BTREE_BASE_H_
#include <tpie/portability.h>
#include <functional>
namespace tpie {

/**
 * \brief Augmentation struct used in an un-augmented btree
 */
struct empty_augment {};

/**
 * \brief Functor used to augment an un-augmented btree
 */
struct empty_augmenter {
	typedef empty_augment value_type;
	
    template <typename T>
    empty_augment operator()(const T &) {return empty_augment();}
};

/**
 * \brief Functor used to extract the key from a value in case 
 * keys and values are the same
 */
struct identity_key {
	template <typename T>
	const T & operator()(const T & t) const noexcept {return t;}
};

/**
 * \brief Default < comparator for the btree
 */
struct default_comp {
	template <typename L, typename R>
	bool operator()(const L & l, const R & r) const noexcept {
		return l < r;
	}
};



namespace bbits {
template <int i>
struct int_opt {static const int O=i;};

static const int f_internal = 1;

} //namespace bbits

template <typename T>
struct btree_comp {typedef T type;};

template <typename T>
struct btree_key {typedef T type;};

template <typename T>
struct btree_augment {typedef T type;};

template <int a_, int b_>
struct btree_fanout {
	static const int a = a_;
	static const int b = b_;
};

using btree_internal = bbits::int_opt<bbits::f_internal>;
using btree_external = bbits::int_opt<0>;

namespace bbits {

template <int O_, int a_, int b_, typename C_, typename K_, typename A_>
struct Opt {
	static const int O=O_;
	static const int a=a_;
	static const int b=b_;
	typedef C_ C;
	typedef K_ K;
	typedef A_ A;
};

template <typename ... Opt>
struct OptComp {};

template <>
struct OptComp<> {
	typedef Opt<0, 0, 0, default_comp, identity_key, empty_augmenter> type;
};

template <int i, typename ... T>
struct OptComp<int_opt<i> , T...> {
	typedef typename OptComp<T...>::type P;
	typedef Opt<P::O | i, P::a, P::b, typename P::C, typename P::K, typename P::A> type;
};

template <typename C, typename ... T>
struct OptComp<btree_comp<C> , T...> {
	typedef typename OptComp<T...>::type P;
	typedef Opt<P::O, P::a, P::b, C, typename P::K, typename P::A> type;
};

template <typename K, typename ... T>
struct OptComp<btree_key<K> , T...> {
	typedef typename OptComp<T...>::type P;
	typedef Opt<P::O, P::a, P::b, typename P::C, K, typename P::A> type;
};

template <typename A, typename ... T>
struct OptComp<btree_augment<A> , T...> {
	typedef typename OptComp<T...>::type P;
	typedef Opt<P::O, P::a, P::b, typename P::C, typename P::K, A> type;
};

template <int a, int b, typename ... T>
struct OptComp<btree_fanout<a, b> , T...> {
	typedef typename OptComp<T...>::type P;
	typedef Opt<P::O, a, b, typename P::C, typename P::K, typename P::A> type;
};

} //namespace bbits



/**
 * Type that is useful for navigating a btree
 *
 * S is the type of the store used
 */
template <typename S>
class btree_node;


template <typename S>
class btree_iterator;

/**
 * \brief Augmented btree
 * 
 * The fanout and location of nodes is decided by the store type S
 * C is the item comparator type
 * A is the type of the augmentation computation fuctor
 */
template <typename T,
          typename O>
class btree_;

/**
 * \brief Augmented btree builder
 *
 * The fanout and location of nodes is decided by the store type S
 * C is the item comparator type
 * A is the type of the augmentation computation fuctor
 */
template <typename T, typename O>
class btree_builder_;

template <typename T, typename A=empty_augment, typename K=identity_key, std::size_t a=2, std::size_t b=4>
class btree_internal_store;

template <typename T, typename A=empty_augment, typename K=identity_key>
class btree_external_store;


} //namespace tpie
#endif /*_TPIE_BTREE_BASE_H_*/
