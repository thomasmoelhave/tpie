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

#ifndef _TPIE_BTREE_NODE_H_
#define _TPIE_BTREE_NODE_H_

#include <tpie/portability.h>
#include <tpie/btree/base.h>
#include <vector>

namespace tpie {

/**
 * Type that is usefull for navigating a btree
 *
 * S is the type of the store used
 */
template <typename S>
class btree_node {
public:
	/**
	 * Type of the key of a value
	 */
	typedef typename S::key_type key_type;

	/**
	 * Type of the augment of a set of notes/values
	 */
	typedef typename S::augment_type augment_type;

	/**
	 * Type of values
	 */
	typedef typename S::value_type value_type;

	/**
	 * \brief Check if this note has a parent
	 *
	 * True iff this is not the root
	 */
	bool has_parent() const {
		if (m_is_leaf)
			return !m_path.empty();
		else
			return m_path.size() > 1;
	}

	/**
	 * \brief Move to the parent node
	 *
	 * Requires hasParent()
	 */
	void parent() {
		if (m_is_leaf)
			m_is_leaf = false;
		else
			m_path.pop_back();
	}
		
	/**
	 * \brief Move to the ith child
	 *
	 * Requires !leaf() and i < count()
	 */
	void child(size_t i) {
		assert(!m_is_leaf);
		assert(i < count());
		if (m_path.size() + 1 == m_store->height()) {
			m_is_leaf = true;
			m_leaf = m_store->get_child_leaf(m_path.back(), i);
		} else
			m_path.push_back(m_store->get_child_internal(m_path.back(), i));
	}
	
	/**
	 * \brief Return the parent node
	 *
	 * Requires hasParent()
	 */
	btree_node get_parent() const {
		btree_node n=*this;
		n.parent();
		return n;
	}
	
	/**
	 * \brief Return the ith child note
	 *
	 * Requires !leaf() and i < count()
	 */
	btree_node get_child(size_t i) const {
		btree_node n=*this;
		n.child(i);
		return n;
	}
	
	/**
	 * \brief Return true if this is a leaf note
	 */
	bool leaf() const {
		return m_is_leaf;
	}
	
	/**
	 * \brief Return the augment of the ith child
	 *
	 * Requires !leaf()
	 */
	const augment_type & augment(size_t i) const {
		return m_store->augment(m_path.back(), i);
	}
	
	/**
	 * \brief Return the minimal key of the i'th child
	 */
	key_type min_key(size_t i) const {
		if (m_is_leaf)
			return m_store->min_key(m_leaf, i);
		else
			return m_store->min_key(m_path.back(), i);
	}

	/**
	 * \brief Return the i'th value
	 *
	 * Requires leaf()
	 */
	const value_type & value(size_t i) const {
		assert(m_is_leaf);
		return m_store->get(m_leaf, i);
	}
	
	/**
	 * \brief Return the i'th value
	 *
	 * Requires leaf()
	 */
	value_type & value(size_t i) {
		assert(m_is_leaf);
		return m_store->get(m_leaf, i);
	}
	
	/**
	 * \brief Return the number of children or values
	 */
	size_t count() const {
		if (m_is_leaf)
			return m_store->count(m_leaf);
		else
			return m_store->count(m_path.back());
	}
	
	btree_node(): m_store(NULL) {}
private:
	typedef typename S::leaf_type leaf_type;
	typedef typename S::internal_type internal_type;

	btree_node(S * store, leaf_type root)
		: m_store(store), m_leaf(root), m_is_leaf(true) {
	}

	btree_node(S * store, internal_type root)
		: m_store(store), m_is_leaf(false) {
		m_path.push_back(root);
	}

	S * m_store;
	std::vector<internal_type> m_path;
	leaf_type m_leaf;
	bool m_is_leaf;

	template <typename X, typename Y, typename A>
	friend class btree;
};

} //namespace tpie
#endif //_TPIE_BTREE_NODE_H_
