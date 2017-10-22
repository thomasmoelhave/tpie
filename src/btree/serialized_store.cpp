// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2017 The TPIE development team
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
#include <tpie/btree/serialized_store.h>

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

serialized_store_base::serialized_store_base(const std::string & path, btree_flags flags)
	: m_height(0), m_size(0), m_flags(flags), metadata_offset(0), metadata_size(0), path(path) {

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

		root = h.root;
	}
	
} 



void serialized_store_base::set_flags(btree_flags flags) {
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


std::vector<char> serialized_store_base::do_compress(const serilization_buffer & uncompressed_buffer) const {
	switch (m_flags & btree_flags::compression_mask) {
#ifdef TPIE_HAS_LZ4
	case compress_lz4: {
		int uncompressed_size = uncompressed_buffer.size();
		int max_compressed_size = LZ4_compressBound(uncompressed_size);
		std::vector<char> compressed_buffer(max_compressed_size);
		int compressed_size = LZ4_compress_default(uncompressed_buffer.data(), compressed_buffer.data(), uncompressed_size, max_compressed_size);
		if (compressed_size == 0)
			throw io_exception("B-tree compression failed");

		compressed_buffer.resize(compressed_size);
		return compressed_buffer;
	}
#endif
#ifdef TPIE_HAS_ZSTD
	case compress_zstd: {
		int uncompressed_size = uncompressed_buffer.size();
		int max_compressed_size = ZSTD_compressBound(uncompressed_size);
		std::vector<char> compressed_buffer(max_compressed_size);
		
		int level = (uint64_t)(m_flags & btree_flags::compression_level_mask) >> 8;
		if (level == 0) level = 5;
		
		int compressed_size = ZSTD_compress(compressed_buffer.data(), max_compressed_size, uncompressed_buffer.data(), uncompressed_size, level);
		if (compressed_size == 0)
			throw io_exception("B-tree compression failed");
		compressed_buffer.resize(compressed_size);
		return compressed_buffer;
	}
#endif
#ifdef TPIE_HAS_SNAPPY
	case compress_snappy: {
		int uncompressed_size = uncompressed_buffer.size();
		int max_compressed_size = snappy::MaxCompressedLength(uncompressed_size);
		std::vector<char> compressed_buffer(max_compressed_size);
		size_t compressed_size = max_compressed_size;
		snappy::RawCompress(uncompressed_buffer.data(), uncompressed_size,
							compressed_buffer.data(), &compressed_size);
		if (compressed_size == 0)
			throw io_exception("B-tree compression failed");
		compressed_buffer.resize(compressed_size);
		return compressed_buffer;
		}
#endif
	case btree_flags::compress_none:
	default:
		throw exception("Unknown compressios, this code shouldn't be reachable");
	}
}


serialized_store_base::serilization_buffer serialized_store_base::do_decompress(int uncompressed_size, const std::vector<char> & in) const {
	serilization_buffer uncompressed_buffer(uncompressed_size);
	switch (m_flags & btree_flags::compression_mask) {
#ifdef TPIE_HAS_LZ4
	case compress_lz4: {
		int r = LZ4_decompress_safe(in.data(), uncompressed_buffer.data(), in.size(), uncompressed_size);
		if (r != uncompressed_size)
			throw io_exception("B-tree decompression failed");
		break;
	}
#endif
#ifdef TPIE_HAS_ZSTD
	case compress_zstd: {
		size_t r = ZSTD_decompress(uncompressed_buffer.data(), uncompressed_size, in.data(), in.size());
		if ((int)r != uncompressed_size)
			throw io_exception("B-tree decompression failed");
		break;
	}
#endif
#ifdef TPIE_HAS_SNAPPY
	case compress_snappy: {
		uint32_t res = 0;
		bool ok2 = GetUncompressedLength(in.data(), &res);
		if (!ok2 || res != uncompressed_size)
			throw io_exception("B-tree decompression failed");
		bool ok = snappy::RawUncompress(in.data(), in.size(), uncompressed_buffer.data());
		if (!ok)
			throw io_exception("B-tree decompression failed");
		break;
	}
#endif
	case btree_flags::compress_none:
	default:
		throw exception("Unknown compression, this code shouldn't be reachable");
	}
	return uncompressed_buffer;
}


void serialized_store_base::set_metadata(const std::string & data) {
	assert(f->is_open());
	metadata_offset = (stream_size_type)f->tellp();
	metadata_size = data.size();
	f->write(data.c_str(), data.size());
}
	
std::string serialized_store_base::get_metadata() {
	assert(f->is_open());
	if (metadata_offset == 0 || metadata_size == 0)
		return {};
	std::string data(metadata_size, '\0');
	f->read(&data[0], metadata_size);
	return data;
}

void serialized_store_base::finalize_build_inner() {
	header h;
	h.magic = header::good_magic;
	h.version = header::current_version;
	h.root = root;
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
} //namespace bits
} //namespace tpie

