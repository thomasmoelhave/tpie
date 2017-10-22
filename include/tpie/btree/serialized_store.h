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

#ifndef _TPIE_BTREE_SERIALIZED_STORE_H_
#define _TPIE_BTREE_SERIALIZED_STORE_H_

#include <tpie/portability.h>
#include <memory.h>
#include <tpie/exception.h>
#include <tpie/btree/base.h>
#include <tpie/tpie_assert.h>
#include <tpie/serialization2.h>
#include <cstddef>
#include <fstream>

namespace tpie {
namespace bbits {


class serialized_store_base {
protected:
	explicit serialized_store_base(const std::string & path, btree_flags flags=btree_flags::defaults);
	
	class serilization_buffer {
	public:
		serilization_buffer() = default;
		serilization_buffer(size_t n) : m_buffer(n) {}

		void read(char * buf, size_t size) {
			assert(m_index + size <= m_buffer.size());
			memcpy(buf, m_buffer.data() + m_index, size);
			m_index += size;
		}

		void write(const char * buf, size_t size) {
            if (m_index + size > m_buffer.size()) {
				m_buffer.resize(m_index + size);
			}
			memcpy(m_buffer.data() + m_index, buf, size);
			m_index += size;
		}

		size_t size() const noexcept {
			return m_buffer.size();
		}

		const char * data() const noexcept {
			return m_buffer.data();
		}

		char * data() noexcept {
			return m_buffer.data();
		}
	private:
		std::vector<char> m_buffer;
		size_t m_index = 0;
	};

	struct header_v0 {
		/*
		 * Version 0: initial
		 * Version 1: added flags
		 */
		static constexpr uint64_t good_magic = 0x8bbd51bfe5e3d477, current_version = 1;
		uint64_t magic;
		uint64_t version; // 0
		off_t root; // offset of root
		size_t height; // tree height (internal and leaf levels)
		size_t size; // number of items (from btree)
		off_t metadata_offset;
		off_t metadata_size;
	};

	struct header : header_v0 {
		btree_flags flags;
	};
	
	
	std::vector<char> do_compress(const serilization_buffer & in) const;
	serilization_buffer do_decompress(int uncompressed_size, const std::vector<char> & in) const;
	void set_metadata(const std::string & data);
	void finalize_build_inner();
	
	size_t m_height;
	size_t m_size;
	btree_flags m_flags;
	off_t metadata_offset, metadata_size;
	off_t root;
	std::string path;
	std::unique_ptr<std::fstream> f;
public:
	std::string get_metadata();

	void set_flags(btree_flags flags);

	size_t height() const noexcept {
		return m_height;
	}

	void set_height(size_t height) noexcept {
		m_height = height;
	}

	size_t size() const noexcept {
		return m_size;
	}

	void set_size(size_t size) noexcept {
		m_size = size;
	}

};

/**
 * \brief Serializing store
 *
 * \tparam T the type of value stored
 * \tparam A the type of augmentation
 * \tparam a the minimum fanout of a node
 * \tparam b the maximum fanout of a node
 */
template <typename T,
		  typename A,
		  std::size_t a_,
		  std::size_t b_,
		  std::size_t bs_
		  >
class serialized_store: public serialized_store_base {
public:
	/**
	 * \brief Type of value of items stored
	 */
	typedef T value_type;

	/**
	 * \brief Type of augmentation stored
	 */
	typedef A augment_type;


	typedef size_t size_type;
	
	typedef uint64_t off_t;


	serialized_store(const serialized_store & o) = delete;
	serialized_store & operator=(const serialized_store & o) = delete;
	serialized_store(serialized_store && o) = default;
	
	serialized_store & operator=(serialized_store && o) {
		this->~serialized_store();
		new (this) serialized_store(o);
		return this;
	}
	
private:
	struct internal_content {
		off_t offset;
		A augment;
		
		static const bool is_trivially_serializable=true;
	};

	static constexpr size_t block_size() {return bs_?bs_:24*1024;}
	static constexpr size_t min_internal_size() {return 1;}
	static constexpr size_t max_internal_size() {return a_ ? a_ : (block_size()  - sizeof(off_t) - sizeof(size_t)) / sizeof(internal_content) ; }
	static constexpr size_t min_leaf_size() {return 1;}
	static constexpr size_t max_leaf_size() {return b_ ? b_ : (block_size() - sizeof(off_t) - sizeof(size_t)) / sizeof(T);}


	template <typename S, typename N>
	void serialize(S & s, const N & i) const {
		using tpie::serialize;

		if ((m_flags & btree_flags::compression_mask) == btree_flags::compress_none) {
			serialize(s, i.count);
			serialize(s, i.values, i.values + i.count);
		} else {
			serilization_buffer uncompressed_buffer(sizeof(i.count) + sizeof(*i.values) * i.count);
			serialize(uncompressed_buffer, i.count);
			serialize(uncompressed_buffer, i.values, i.values + i.count);
			std::vector<char> compressed_buffer = do_compress(uncompressed_buffer);
			int uncompressed_size = uncompressed_buffer.size();
			int compressed_size = compressed_buffer.size();
			s.write(reinterpret_cast<const char *>(&uncompressed_size), sizeof(uncompressed_size));
			s.write(reinterpret_cast<const char *>(&compressed_size), sizeof(compressed_size));
			s.write(compressed_buffer.data(), compressed_size);
		}
	}
		
	
	template <typename D, typename N>
	void unserialize(D & d, N & i) const {
		using tpie::unserialize;

		if ((m_flags & btree_flags::compression_mask) == btree_flags::compress_none) {
			unserialize(d, i.count);
			unserialize(d, i.values, i.values + i.count);
		} else {
			int uncompressed_size, compressed_size;
			d.read(reinterpret_cast<char *>(&uncompressed_size), sizeof(uncompressed_size));
			d.read(reinterpret_cast<char *>(&compressed_size), sizeof(compressed_size));
			std::vector<char> compressed_buffer(compressed_size);
			d.read(compressed_buffer.data(), compressed_size);
			serilization_buffer uncompressed_buffer = do_decompress(uncompressed_size, compressed_buffer);

			unserialize(uncompressed_buffer, i.count);
			unserialize(uncompressed_buffer, i.values, i.values + i.count);
		}
	}
		

	struct internal {
		off_t my_offset; //NOTE not serialized
		size_t count;
		internal_content values[max_internal_size()];
	};

	struct leaf {
		off_t my_offset; //NOTE not serialized
		size_t count;
		T values[max_leaf_size()];
	};
	
	typedef std::shared_ptr<internal> internal_type;
	typedef std::shared_ptr<leaf> leaf_type;

	/**
	 * \brief Construct a new empty btree storage
	 *
	 * flags are currently ignored when write_only is false
	 */
	explicit serialized_store(const std::string & path, btree_flags flags=btree_flags::defaults):
		serialized_store_base(path, flags) {
		if (flags & btree_flags::read) {
			if (m_height == 1) {
				root_leaf = std::make_shared<leaf>();
				root_leaf->my_offset = root;
				f->seekg(root);
				unserialize(*f, *root_leaf);
			} else if (m_height > 1) {
				root_internal = std::make_shared<internal>();
				root_internal->my_offset = root;
				f->seekg(root);
				unserialize(*f, *root_internal);
			}
		}
	}

	
	void move(internal_type src, size_t src_i,
			  internal_type dst, size_t dst_i) {
		dst->values[dst_i] = src->values[src_i];
	}

	void move(leaf_type src, size_t src_i,
			  leaf_type dst, size_t dst_i) {
		dst->values[dst_i] = src->values[src_i];
	}
	
	void set(leaf_type dst, size_t dst_i, T c) {
		assert(dst == current_leaf);
		dst->values[dst_i] = c;
	}
		
	void set(internal_type node, size_t i, internal_type c) {
		assert(node == current_internal);
		node->values[i].offset = c->my_offset;
	}

	void set(internal_type node, size_t i, leaf_type c) {
		assert(node == current_internal);
		node->values[i].offset = c->my_offset;
	}

	const T & get(leaf_type l, size_t i) const {
		return l->values[i];
	}

	size_t count(internal_type node) const {
		return node->count;
	}
	
	size_t count(leaf_type node) const {
		return node->count;
	}

	void set_count(internal_type node, size_t i) {
		node->count = i;
	}

	void set_count(leaf_type node, size_t i) {
		node->count = i;
	}

	leaf_type create_leaf() {
		assert(!current_internal && !current_leaf);
		current_leaf = std::make_shared<leaf>();
		current_leaf->my_offset = (stream_size_type)f->tellp();
		return current_leaf;
	}
	leaf_type create(leaf_type) {return create_leaf();}
	internal_type create_internal() {
		assert(!current_internal && !current_leaf);
		current_internal = std::make_shared<internal>();
		current_internal->my_offset = (stream_size_type)f->tellp();
		return current_internal;
	}
	internal_type create(internal_type) {return create_internal();}
	
	void set_root(internal_type node) {root_internal = node;}
	void set_root(leaf_type node) {root_leaf = node;}

	internal_type get_root_internal() const {
		return root_internal;
	}

	leaf_type get_root_leaf() const {
		return root_leaf;
	}

	internal_type get_child_internal(internal_type node, size_t i) const {
		internal_type child = std::make_shared<internal>();
		assert(i < node->count);
		child->my_offset = node->values[i].offset;
		f->seekg(child->my_offset);
		unserialize(*f, *child);
		return child;
	}

	leaf_type get_child_leaf(internal_type node, size_t i) const {
		leaf_type child = std::make_shared<leaf>();
		assert(i < node->count);
		child->my_offset = node->values[i].offset;
		f->seekg(child->my_offset);
		unserialize(*f, *child);
		return child;
	}

	size_t index(off_t my_offset, internal_type node) const {
		for (size_t i=0; i < node->count; ++i)
			if (node->values[i].offset == my_offset) return i;
		tp_assert(false, "Not found");
		tpie_unreachable();
	}
	
	size_t index(leaf_type l, internal_type node) const {
		return index(l->my_offset, node);
	}

	size_t index(internal_type i, internal_type node) const {
		return index(i->my_offset, node);
	}
	
	void set_augment(leaf_type l, internal_type p, augment_type ag) {
		size_t idx = index(l->my_offset, p);
		p->values[idx].augment = ag;
	}
	
	void set_augment(internal_type i, internal_type p, augment_type ag) {
		size_t idx = index(i->my_offset, p);
		p->values[idx].augment = ag;
	}

	const augment_type & augment(internal_type p, size_t i) const {
		return p->values[i].augment;
	}
		
	void flush() {
		if (current_internal) {
			assert(!current_leaf);
			assert((stream_size_type)f->tellp() == current_internal->my_offset);
			serialize(*f, *current_internal);
			current_internal.reset();
		}
		if (current_leaf) {
			assert((stream_size_type)f->tellp() == current_leaf->my_offset);
			serialize(*f, *current_leaf);
			current_leaf.reset();
		}
	}
	
	void finalize_build() {
		// Should call flush() first.
		assert(!current_internal && !current_leaf);
		if (root_internal) {
			root = root_internal->my_offset;
		} else if (root_leaf) {
			root = root_leaf->my_offset;
		} else {
			assert(m_size == 0);
		}
		finalize_build_inner();
	}
	
	void set_metadata(const std::string & data) {
		assert(!current_internal && !current_leaf);
		serialized_store_base::set_metadata(data);
	}

	internal_type current_internal, root_internal;
	leaf_type current_leaf, root_leaf;

	template <typename>
	friend class ::tpie::btree_node;

	template <typename>
	friend class ::tpie::btree_iterator;

	template <typename, typename>
	friend class bbits::tree;

	template <typename, typename>
	friend class bbits::tree_state;

	template<typename, typename>
	friend class bbits::builder;

	template <typename, bool>
	friend struct bbits::block_size_getter;
};

} //namespace bbits
} //namespace tpie
#endif /*_TPIE_BTREE_SERIALIZED_STORE_H_*/
