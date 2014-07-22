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

#ifndef _TPIE_BTREE_INTERNAL_STORE_H_
#define _TPIE_BTREE_INTERNAL_STORE_H_

#include <tpie/portability.h>
#include <tpie/blocks/base.h>
#include <tpie/tpie_assert.h>
#include <cstddef>

namespace tpie {

/**
 * \brief Storrage used for an internal btree
 * 
 * T is the type of value storred
 * A is the type of augmentation
 * K is the functor used to istract the key from a given value
 * a is the minimal fanout of a node
 * b is the maximal fanout of a node
 *
 * Note that a user of a btree should
 * not call the 
 */
template <typename T,
		  typename A=empty_augment,
		  typename K=identity_key<T>,
		  std::size_t a=2,
		  std::size_t b=4
		  >
class btree_internal_store {
public:
	/**
	 * \brief Type of value of items storred
	 */
	typedef T value_type;

	/**
	 * \brief Type of augmentation storred
	 */
	typedef A augment_type;

	/**
	 * \brief Type of functer used to extract a key from a value
	 */
	typedef K key_extract_type;

	/**
	 * \brief Type of key
	 */
	typedef typename K::value_type key_type;

	typedef size_t size_type;
	
	/**
	 * \brief Construct a new empty btree storrage
	 */
	btree_internal_store(K key_extract=K()): 
		m_root(NULL), key_extract(key_extract),
		m_height(0), m_size(0) {}
private:
	struct internal_content {
		key_type min_key;
		void * ptr;
		A augment;
	};

	struct internal {
		size_t count;
		internal_content values[b];
	};
	
	struct leaf {
		size_t count;
		T values[b];
	};

	typedef internal * internal_type;
	typedef leaf * leaf_type;

	static size_t min_size() {return a;}
	static size_t max_size() {return b;}
	
	void move(internal_type src, size_t src_i,
			  internal_type dst, size_t dst_i) {
		dst->values[dst_i] = src->values[src_i];
	}

	void move(leaf_type src, size_t src_i,
			  leaf_type dst, size_t dst_i) {
		dst->values[dst_i] = src->values[src_i];
	}

	void set(leaf_type dst, size_t dst_i, T c) {
		dst->values[dst_i] = c;
	}
		
	void set(internal_type node, size_t i, internal_type c) {
		node->values[i].ptr = c;
	}

	void set(internal_type node, size_t i, leaf_type c) {
		node->values[i].ptr = c;
	}

	T & get(leaf_type l, size_t i) {
		return l->values[i];
	}

	size_t count(internal_type node) {
		return node->count;
	}

	size_t count_child_leaf(internal_type node, size_t i) {
		return static_cast<leaf_type>(node->values[i].ptr)->count;
	}

	size_t count_child_internal(internal_type node, size_t i) {
		return static_cast<internal_type>(node->values[i].ptr)->count;
	}

	size_t count(leaf_type node) {
		return node->count;
	}

	void set_count(internal_type node, size_t i) {
		node->count = i;
	}

	void set_count(leaf_type node, size_t i) {
		node->count = i;
	}

	key_type min_key(internal_type node, size_t i) {
		return node->values[i].min_key;
	}

	key_type min_key(leaf_type node, size_t i) {
		return key_extract(node->values[i]);
	}

	key_type min_key(T v) {
		return key_extract(v);
	}

	key_type min_key(internal_type v) {
		return min_key(v, 0);
	}

	key_type min_key(leaf_type v) {
		return min_key(v, 0);
	}

	leaf_type create_leaf() {return new leaf();}
	leaf_type create(leaf_type) {return create_leaf();}
	internal_type create_internal() {return new internal();}
	internal_type create(internal_type) {return create_internal();}

	void destroy(internal_type node) {delete node;}
	void destroy(leaf_type node) {delete node;}

	void set_root(internal_type node) {m_root = node;}
	void set_root(leaf_type node) {m_root = node;}

	internal_type get_root_internal() {return static_cast<internal_type>(m_root);}
	leaf_type get_root_leaf() {return static_cast<leaf_type>(m_root);}
	internal_type get_child_internal(internal_type node, size_t i) {
		return static_cast<internal_type>(node->values[i].ptr);
	}
	leaf_type get_child_leaf(internal_type node, size_t i) {
		return static_cast<leaf_type>(node->values[i].ptr);
	}

	size_t index(leaf_type child, internal_type node) {
		for (size_t i=0; i < node->count; ++i)
			if (node->values[i].ptr == child) return i;
		tp_assert(false, "Leaf not found");
		__builtin_unreachable();
	}

	size_t index(internal_type child, internal_type node) {
		for (size_t i=0; i < node->count; ++i)
			if (node->values[i].ptr == child) return i;
		tp_assert(false, "Node nout found");
		__builtin_unreachable();
	}

	void set_augment(leaf_type l, internal_type p, augment_type ag) {
		size_t idx=index(l, p);
		p->values[idx].min_key = min_key(l);
		p->values[idx].augment = ag;
		
	}

	void set_augment(internal_type l, internal_type p, augment_type ag) {
		size_t idx=index(l, p);
		p->values[idx].min_key = min_key(l);
		p->values[idx].augment = ag;
	}

	const augment_type & augment(internal_type p, size_t i) {
		return p->values[i].augment;
	}
	
	size_t height() const {return m_height;}
	void set_height(size_t height) {m_height = height;}

	size_t size() const {return m_size;}
	void set_size(size_t size) {m_size = size;}

	void * m_root;
	K key_extract;
	size_t m_height;
	size_t m_size;

	template <typename>
	friend class btree_node;

	template <typename>
	friend class btree_iterator;

	template <typename, typename, typename>
	friend class btree;
};

} //namespace tpie
#endif /*_TPIE_BTREE_INTERNAL_STORE_H_*/
