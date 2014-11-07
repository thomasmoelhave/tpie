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

#ifndef _TPIE_BTREE_EXTERNAL_STORE_H_
#define _TPIE_BTREE_EXTERNAL_STORE_H_

#include <tpie/portability.h>
#include <tpie/blocks/base.h>
#include <tpie/tpie_assert.h>
#include <tpie/blocks/block_collection_cache.h>
#include <boost/shared_ptr.hpp>

#include <cstddef>

namespace tpie {

/**
 * \brief Storage used for an external btree
 * 
 * T is the type of value stored
 * A is the type of augmentation
 * K is the functor used to extract the key from a given value
 * a is the minimum fanout of a node
 * b is the maximum fanout of a node
 *
 * Note that a user of a btree should
 * not call the store directly
 */
template <typename T,
		  typename A=empty_augment,
		  typename K=identity_key<T>
		  >
class btree_external_store {
public:
	/**
	 * \brief Type of value of items stored
	 */
	typedef T value_type;

	/**
	 * \brief Type of augmentation stored
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

	static const memory_size_type cacheSize = 16;
private:

	struct internal_content {
		key_type min_key;
		blocks::block_handle handle;
		A augment;
	};

	struct internal {
		internal(blocks::block & b) {
			count = reinterpret_cast<memory_size_type *>(b.get());
			values = reinterpret_cast<internal_content *>(b.get() + sizeof(memory_size_type));
		}

		memory_size_type * count;
		internal_content * values;
	};
	
	struct leaf {
		leaf(blocks::block & b) {
			count = reinterpret_cast<memory_size_type *>(b.get());
			values = reinterpret_cast<T *>(b.get() + sizeof(memory_size_type));
		}

		memory_size_type * count;
		T * values;
	};

	struct internal_type {
		internal_type() {}
		internal_type(blocks::block_handle handle) : handle(handle) {}

		blocks::block_handle handle;

		bool operator==(const internal_type & other) const {
			return handle == other.handle;
		}
	};

	struct leaf_type {
		leaf_type() {}
		leaf_type(blocks::block_handle handle) : handle(handle) {}

		blocks::block_handle handle;

		bool operator==(const leaf_type & other) const {
			return handle == other.handle;
		}
	};
public:

	/**
	 * \brief Construct a new empty btree storage
	 */
	btree_external_store(const std::string & path, K key_extract=K())
		: m_root()
		, key_extract(key_extract)
		, m_height(0)
		, m_size(0) 
		{
			memory_size_type blockSize = get_block_size();
			m_nodeMax = (blockSize - sizeof(memory_size_type)) / sizeof(internal_content);
			m_nodeMin = (m_nodeMax + 3) / 4;
			m_leafMax = (blockSize - sizeof(memory_size_type)) / sizeof(T);
			m_leafMin = (m_leafMax + 3) / 4;
			m_collection.reset(new blocks::block_collection_cache(path, true, blockSize * cacheSize));
		}
private:

	static size_t min_size() {
		return (max_size() + 3) / 4;
	}

	static size_t max_size() {
		return (get_block_size() - sizeof(memory_size_type)) / sizeof(internal_content);
	}
	
	void move(internal_type src, size_t src_i,
			  internal_type dst, size_t dst_i) {
		blocks::block srcBlock, dstBlock;
		m_collection->read_block(src.handle, srcBlock);
		m_collection->read_block(dst.handle, dstBlock);

		internal srcInter(srcBlock);
		internal dstInter(dstBlock);

		dstInter.values[dst_i] = srcInter.values[src_i];

		m_collection->write_block(src.handle, srcBlock);
		m_collection->write_block(dst.handle, dstBlock);
	}

	void move(leaf_type src, size_t src_i,
			  leaf_type dst, size_t dst_i) {
		blocks::block srcBlock, dstBlock;
		m_collection->read_block(src.handle, srcBlock);
		m_collection->read_block(dst.handle, dstBlock);

		leaf srcInter(srcBlock);
		leaf dstInter(dstBlock);

		dstInter.values[dst_i] = srcInter.values[src_i];

		m_collection->write_block(src.handle, srcBlock);
		m_collection->write_block(dst.handle, dstBlock);
	}

	void set(leaf_type dst, size_t dst_i, T c) {
		blocks::block dstBlock;
		m_collection->read_block(dst.handle, dstBlock);
		leaf dstInter(dstBlock);

		dstInter.values[dst_i] = c;

		m_collection->write_block(dst.handle, dstBlock);
	}
		
	void set(internal_type node, size_t i, internal_type c) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		internal nodeInter(nodeBlock);

		nodeInter.values[i].handle = c.handle;

		m_collection->write_block(node.handle, nodeBlock);
	}

	void set(internal_type node, size_t i, leaf_type c) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		internal nodeInter(nodeBlock);

		nodeInter.values[i].handle = c.handle;

		m_collection->write_block(node.handle, nodeBlock);
	}

	T & get(leaf_type node, size_t i) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		leaf nodeInter(nodeBlock);

		return nodeInter.values[i];
	}

	size_t count(internal_type node) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		internal nodeInter(nodeBlock);

		return *(nodeInter.count);
	}

	size_t count(leaf_type node) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		leaf nodeInter(nodeBlock);

		return *(nodeInter.count);
	}

	size_t count_child_leaf(internal_type node, size_t i) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		internal nodeInter(nodeBlock);

		leaf_type wrap(nodeInter.values[i].handle);
		return count(wrap);	
	}

	size_t count_child_internal(internal_type node, size_t i) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		internal nodeInter(nodeBlock);

		internal_type wrap(nodeInter.values[i].handle);
		return count(wrap);
	}

	void set_count(internal_type node, size_t i) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		internal nodeInter(nodeBlock);

		*(nodeInter.count) = i;

		m_collection->write_block(node.handle, nodeBlock);
	}

	void set_count(leaf_type node, size_t i) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		leaf nodeInter(nodeBlock);

		*(nodeInter.count) = i;

		m_collection->write_block(node.handle, nodeBlock);
	}

	key_type min_key(internal_type node, size_t i) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		internal nodeInter(nodeBlock);

		return nodeInter.values[i].min_key;
	}

	key_type min_key(leaf_type node, size_t i) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		leaf nodeInter(nodeBlock);

		return key_extract(nodeInter.values[i]);
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

	leaf_type create_leaf() {
		blocks::block_handle h = m_collection->get_free_block(get_block_size());
		blocks::block b(h.size);
		leaf l(b);
		(*l.count) = 0;
		m_collection->write_block(h, b);
		return leaf_type(h);
	}

	leaf_type create(leaf_type) {
		return create_leaf();
	}

	internal_type create_internal() {
		blocks::block_handle h = m_collection->get_free_block(get_block_size());
		blocks::block b(h.size);
		internal i(b);
		(*i.count) = 0;
		m_collection->write_block(h, b);
		return internal_type(h);
	}

	internal_type create(internal_type) {
		return create_internal();
	}

	void destroy(internal_type node) {
		m_collection->free_block(node.handle);
	}

	void destroy(leaf_type node) {
		m_collection->free_block(node.handle);
	}

	void set_root(internal_type node) {
		m_root = node.handle;
	}

	void set_root(leaf_type node) {
		m_root = node.handle;
	}

	internal_type get_root_internal() {
		return internal_type(m_root);
	}

	leaf_type get_root_leaf() {
		return leaf_type(m_root);
	}

	internal_type get_child_internal(internal_type node, size_t i) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		internal dstInter(nodeBlock);

		return internal_type(dstInter.values[i].handle);
	}

	leaf_type get_child_leaf(internal_type node, size_t i) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		internal dstInter(nodeBlock);

		return leaf_type(dstInter.values[i].handle);
	}

	size_t index(leaf_type child, internal_type node) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		internal dstInter(nodeBlock);

		for (size_t i=0; i < *(dstInter.count); ++i)
			if (dstInter.values[i].handle == child.handle) return i;
		tp_assert(false, "Leaf not found");
		__builtin_unreachable();
	}

	size_t index(internal_type child, internal_type node) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		internal dstInter(nodeBlock);

		for (size_t i=0; i < *(dstInter.count); ++i)
			if (dstInter.values[i].handle == child.handle) return i;
		tp_assert(false, "Node not found");
		__builtin_unreachable();
	}

	void set_augment(leaf_type child, internal_type node, augment_type augment) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		internal nodeInter(nodeBlock);

		for (size_t i=0; i < *(nodeInter.count); ++i)
		{
			if (nodeInter.values[i].handle == child.handle) {
				nodeInter.values[i].min_key = min_key(child);
				nodeInter.values[i].augment = augment;
				m_collection->write_block(node.handle, nodeBlock);
				return;
			}
		}

		tp_assert(false, "Leaf not found");
		__builtin_unreachable();
	}

	void set_augment(internal_type child, internal_type node, augment_type augment) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		internal nodeInter(nodeBlock);

		for (size_t i=0; i < *(nodeInter.count); ++i)
		{
			if (nodeInter.values[i].handle == child.handle) {
				nodeInter.values[i].min_key = min_key(child);
				nodeInter.values[i].augment = augment;
				m_collection->write_block(node.handle, nodeBlock);
				return;
			}
		}
		
		tp_assert(false, "Node not found");
		__builtin_unreachable();
	}

	const augment_type & augment(internal_type node, size_t i) {
		blocks::block nodeBlock;
		m_collection->read_block(node.handle, nodeBlock);
		internal nodeInter(nodeBlock);

		return nodeInter.values[i].augment;
	}
	
	size_t height() const {
		return m_height;
	}

	void set_height(size_t height) {
		m_height = height;
	}

	size_t size() const {
		return m_size;
	}

	void set_size(size_t size) {
		m_size = size;
	}

	blocks::block_handle m_root;
	K key_extract;
	size_t m_height;
	size_t m_size;
	boost::shared_ptr<blocks::block_collection_cache> m_collection;

	memory_size_type m_nodeMax;
	memory_size_type m_nodeMin;
	memory_size_type m_leafMax;
	memory_size_type m_leafMin;

	template <typename>
	friend class btree_node;

	template <typename>
	friend class btree_iterator;

	template <typename, typename, typename>
	friend class btree;
};

} //namespace tpie
#endif /*_TPIE_BTREE_EXTERNAL_STORE_H_*/
