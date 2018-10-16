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
#include <tpie/btree/base.h>
#include <tpie/tpie_assert.h>
#include <tpie/serialization2.h>
#include <cstddef>
#include <fstream>

#ifdef TPIE_HAS_LZ4
#include <lz4.h>
#endif
#ifdef TPIE_HAS_ZSTD
#include <zstd.h>
#endif
#ifdef TPIE_HAS_SNAPPY
#include <snappy.h>
#endif

namespace tpie {
namespace bbits {

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
class serialized_store {
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
	};
	static_assert(std::is_trivially_copyable<internal_content>::value, "must be trivially copyable.");

	static constexpr size_t block_size() {return bs_?bs_:24*1024;}
	static constexpr size_t min_internal_size() {return 1;}
	static constexpr size_t max_internal_size() {return a_ ? a_ : (block_size()  - sizeof(off_t) - sizeof(size_t)) / sizeof(internal_content) ; }
	static constexpr size_t min_leaf_size() {return 1;}
	static constexpr size_t max_leaf_size() {return b_ ? b_ : (block_size() - sizeof(off_t) - sizeof(size_t)) / sizeof(T);}

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

		size_t size() {
			return m_buffer.size();
		}

		char * data() {
			return m_buffer.data();
		}

	private:
		std::vector<char> m_buffer;
		size_t m_index = 0;
	};

	template <typename S, typename N>
	void serialize(S & s, const N & i) const {
		using tpie::serialize;

		auto compression_type = m_flags & btree_flags::compression_mask;

		if (compression_type == btree_flags::compress_none) {
			serialize(s, i.count);
			serialize(s, i.values, i.values + i.count);
			return;
		}

		serilization_buffer uncompressed_buffer(sizeof(i.count) + sizeof(*i.values) * i.count);
		serialize(uncompressed_buffer, i.count);
		serialize(uncompressed_buffer, i.values, i.values + i.count);

		int32_t uncompressed_size = (int32_t)uncompressed_buffer.size();

		std::vector<char> compressed_buffer;
		int32_t compressed_size;

		switch (compression_type) {
#ifdef TPIE_HAS_LZ4
			case btree_flags::compress_lz4: {
				auto max_compressed_size = LZ4_compressBound(uncompressed_size);
				compressed_buffer.resize((size_t)max_compressed_size);

				compressed_size = LZ4_compress_default(uncompressed_buffer.data(), compressed_buffer.data(),
				                                       uncompressed_size, max_compressed_size);
				if (compressed_size == 0)
					throw io_exception("B-tree compression failed");

				break;
			}
#endif
#ifdef TPIE_HAS_ZSTD
				case compress_zstd: {
					auto max_compressed_size = ZSTD_compressBound((size_t)uncompressed_size);
					compressed_buffer.resize(max_compressed_size);

					int level = (int)((uint64_t)(m_flags & btree_flags::compression_level_mask) >> 8);
					if (level == 0) level = 5;

					size_t r = ZSTD_compress(compressed_buffer.data(), max_compressed_size, uncompressed_buffer.data(), uncompressed_size, level);
					if (ZSTD_isError(r))
						throw io_exception("B-tree compression failed");

					compressed_size = (int32_t)r;
					break;
				}
#endif
#ifdef TPIE_HAS_SNAPPY
			case compress_snappy: {
				auto max_compressed_size = snappy::MaxCompressedLength((size_t) uncompressed_size);
				compressed_buffer.resize(max_compressed_size);

				size_t _compressed_size;
				snappy::RawCompress(uncompressed_buffer.data(), uncompressed_size,
				                    compressed_buffer.data(), &_compressed_size);

				compressed_size = (int32_t) _compressed_size;
				break;
			}
#endif
			default:
				throw exception("Unknown compression, this code shouldn't be reachable");
		}

		s.write(reinterpret_cast<char *>(&uncompressed_size), sizeof(uncompressed_size));
		s.write(reinterpret_cast<char *>(&compressed_size), sizeof(compressed_size));
		s.write(compressed_buffer.data(), compressed_size);
	}

	template <typename D, typename N>
	void unserialize(D & d, N & i) const {
		using tpie::unserialize;

		auto compression_type = m_flags & btree_flags::compression_mask;

		if (compression_type == btree_flags::compress_none) {
			unserialize(d, i.count);
			unserialize(d, i.values, i.values + i.count);
			return;
		}

		int32_t uncompressed_size, compressed_size;
		d.read(reinterpret_cast<char *>(&uncompressed_size), sizeof(uncompressed_size));
		d.read(reinterpret_cast<char *>(&compressed_size), sizeof(compressed_size));

		std::vector<char> compressed_buffer((size_t)compressed_size);
		d.read(compressed_buffer.data(), compressed_size);

		serilization_buffer uncompressed_buffer((size_t)uncompressed_size);

		switch (m_flags & btree_flags::compression_mask) {
#ifdef TPIE_HAS_LZ4
		case compress_lz4: {
			int r = LZ4_decompress_fast(compressed_buffer.data(), uncompressed_buffer.data(), uncompressed_size);
			if (r != compressed_size)
				throw io_exception("B-tree decompression failed");
			break;
		}
#endif
#ifdef TPIE_HAS_ZSTD
		case compress_zstd: {
			size_t r = ZSTD_decompress(uncompressed_buffer.data(), (size_t)uncompressed_size, compressed_buffer.data(), (size_t)compressed_size);
			if (ZSTD_isError(r) || (int32_t)r != uncompressed_size)
				throw io_exception("B-tree decompression failed");
			break;
		}
#endif
#ifdef TPIE_HAS_SNAPPY
		case compress_snappy: {
			bool ok = snappy::RawUncompress(compressed_buffer.data(), (size_t)compressed_size, uncompressed_buffer.data());
			if (!ok)
				throw io_exception("B-tree decompression failed");
			break;
		}
#endif
		default:
            throw exception("Unknown compression, this code shouldn't be reachable");
		}

		unserialize(uncompressed_buffer, i.count);
		unserialize(uncompressed_buffer, i.values, i.values + i.count);
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

	class leaf_type {
		std::shared_ptr<leaf> ptr;

	public:
		leaf_type() = default;
		leaf_type(const leaf_type &) = default;
		leaf_type(leaf_type &&) = default;

		leaf_type & operator=(const leaf_type &) = default;
		leaf_type & operator=(leaf_type &&) = default;

		leaf_type(off_t offset) {
			ptr = std::make_shared<leaf>();
			ptr->my_offset = offset;
		}

		leaf & operator*() const noexcept {
			return *ptr;
		}

		leaf * operator->() const noexcept {
			return ptr.operator->();
		}

		void reset() noexcept {
			ptr.reset();
		}

		explicit operator bool() const noexcept {
			return bool(ptr);
		}

		bool operator==(const leaf_type & o) const noexcept {
			return ptr->my_offset == o->my_offset;
		}

		bool operator!=(const leaf_type & o) const noexcept {
			return !(this == o);
		}
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
	
	typedef std::shared_ptr<internal> internal_type;

	void set_flags(btree_flags flags) {
		m_flags = flags;
		switch (flags & btree_flags::compression_mask) {
		case btree_flags::compress_lz4:
#ifndef TPIE_HAS_LZ4
			throw exception("Can't use a LZ4 compressed B-tree without LZ4 installed");
#endif
			break;
		case btree_flags::compress_zstd:
#ifndef TPIE_HAS_ZSTD
			throw exception("Can't use a ZSTD compressed B-tree without ZSTD installed");
#endif
			break;
		case btree_flags::compress_snappy:
#ifndef TPIE_HAS_SNAPPY
			throw exception("Can't use a snappy compressed B-tree without snappy installed");
#endif
			break;
		case btree_flags::compress_none:
			break;
		default:
			throw exception("Unknown compression");
		}
	}
	
	/**
	 * \brief Construct a new empty btree storage
	 *
	 * flags are currently ignored when write_only is false
	 */
	explicit serialized_store(const std::string & path, btree_flags flags=btree_flags::defaults):
		m_height(0), m_size(0), metadata_offset(0), metadata_size(0), path(path) {
		f.reset(new std::fstream());
		header h;
		if ((flags & btree_flags::read) == 0) {
			if ((flags & btree_flags::write) == 0)
				throw invalid_file_exception("Either read or write must be supplied to serialized store");
			f->open(path, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
			if (!f->is_open())
				throw invalid_file_exception("Open failed");
			memset(&h, 0, sizeof(h));
			h.flags = flags;
			set_flags(flags);
			f->write(reinterpret_cast<char *>(&h), sizeof(h));
		} else {
			f->open(path, std::ios_base::in | std::ios_base::binary);
			if (!f->is_open())
				throw invalid_file_exception("Open failed");
			f->read(reinterpret_cast<char *>(&h), sizeof(header_v0));
			if (!*f) 
				throw invalid_file_exception("Unable to read header");
			
			if (h.magic != header::good_magic) 
				throw invalid_file_exception("Bad magic");

			if (h.version == 0) {
				h.flags = btree_flags::defaults_v0;
			} else if (h.version == 1) {
				f->read(reinterpret_cast<char *>(&h.flags), sizeof(h.flags));
			} else {
				throw invalid_file_exception("Bad version");
			}

			m_height = h.height;
			m_size = h.size;
			metadata_offset = h.metadata_offset;
			metadata_size = h.metadata_size;

            set_flags(h.flags);

			if (m_height == 1) {
				root_leaf = leaf_type(h.root);
				f->seekg(h.root);
				unserialize(*f, *root_leaf);
			} else if (m_height > 1) {
				root_internal = std::make_shared<internal>();
				root_internal->my_offset = h.root;
				f->seekg(h.root);
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
		current_leaf = leaf_type((off_t)f->tellp());
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
		leaf_type child = leaf_type(node->values[i].offset);
		assert(i < node->count);
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
	
	size_t height() const throw() {
		return m_height;
	}

	void set_height(size_t height) throw() {
		m_height = height;
	}

	size_t size() const throw() {
		return m_size;
	}

	void set_size(size_t size) throw() {
		m_size = size;
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

		header h;
		h.magic = header::good_magic;
		h.version = header::current_version;
		h.root = 0;
		if (root_internal) {
			h.root = root_internal->my_offset;
		} else if (root_leaf) {
			h.root = root_leaf->my_offset;
		} else {
			assert(m_size == 0);
		}
		h.height = m_height;
		h.size = m_size;
		h.metadata_offset = metadata_offset;
		h.metadata_size = metadata_size;
		h.flags = m_flags;
		f->seekp(0);
		f->write(reinterpret_cast<char *>(&h), sizeof(h));
		f->close();
		
		f->open(path, std::ios_base::in | std::ios_base::binary);
		if (!f->is_open())
			throw invalid_file_exception("Open failed");
	}
	
	void set_metadata(const std::string & data) {
		assert(!current_internal && !current_leaf);
		assert(f->is_open());
		metadata_offset = (stream_size_type)f->tellp();
		metadata_size = data.size();
		f->write(data.c_str(), data.size());
	}
	
	std::string get_metadata() {
		assert(f->is_open());
		if (metadata_offset == 0 || metadata_size == 0)
			return {};
		std::string data(metadata_size, '\0');
		f->read(&data[0], metadata_size);
		return data;
	}

	size_t m_height;
	size_t m_size;
	off_t metadata_offset, metadata_size;
	btree_flags m_flags;
	
	std::string path;
	std::unique_ptr<std::fstream> f;
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
