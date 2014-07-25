// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet cino+=(0 :
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

///////////////////////////////////////////////////////////////////////////////
/// \file block_collection Class to handle blocks of varying size on disk
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_BLOCK_COLLECTION_H
#define _TPIE_BLOCK_COLLECTION_H

#include <tpie/tpie.h>
#include <tpie/tpie_assert.h>
#include <tpie/array.h>
#include <tpie/file_accessor/file_accessor.h>
#include <tpie/blocks/block.h>
#include <map>
#include <limits>

namespace tpie {

namespace blocks {

namespace bits {
	class freespace_collection {
	private:
		static const memory_size_type alignment = 32; // the block sizes are a multiple of alignment(the smallest block size is therefore alignment)

		struct position_comparator {
			bool operator()(const block_handle & a, const block_handle & b) {
				return a.position < b.position;
			}
		};

		typedef std::map<stream_size_type, stream_size_type> size_map_t;
		typedef std::map<block_handle, size_map_t::iterator, position_comparator> position_map_t;
	public:
		freespace_collection() {
			// the initial configuration is an empty block with the maximum possible size
			stream_size_type size = std::numeric_limits<stream_size_type>::max();
			stream_size_type pos = 0;

			size_map_t::iterator i = m_blockSizeMap.insert(std::make_pair(size, pos)).first;
			m_blockPositionMap.insert(std::make_pair(block_handle(pos, size), i));
		}

		void free(block_handle handle) {
			if(!m_blockPositionMap.empty()) { // try to find adjacent blocks if the map is not empty
				position_map_t::iterator i = m_blockPositionMap.upper_bound(handle); // find the first block whose position is greather than that of block handle

				if(i != m_blockPositionMap.begin()) {
					position_map_t::iterator j = i;	--j; // find the last block whose position is less than that of block handle
					block_handle prev = j->first;
					if(prev.position + prev.size == handle.position) { // merge the two blocks if they are adjacent
						handle.position = prev.position;
						handle.size += prev.size;
						m_blockSizeMap.erase(j->second);
						m_blockPositionMap.erase(j);
					}
				}

				if(i != m_blockPositionMap.end()) {
					block_handle next = i->first;
					if(handle.position + handle.size == next.position) { // merge the two blocks if they are adjacent
						handle.size += next.size;
						m_blockSizeMap.erase(i->second);
						m_blockPositionMap.erase(i);
					}
				}
			}

			size_map_t::iterator i = m_blockSizeMap.insert(std::make_pair(handle.size, handle.position)).first;
			m_blockPositionMap.insert(std::make_pair(handle, i)); // update the block map to contain the now free block
		}

		block_handle alloc(stream_size_type size) {
			tp_assert(size > 0, "The block size should be greather than zero");

			stream_size_type mod = size % alignment;
			if(mod != 0) // ceil to nearest multiple of alignment
				size += alignment - mod;

			// find the best fit empty block and remove it from the two maps
			size_map_t::iterator i = m_blockSizeMap.lower_bound(size);
			position_map_t::iterator j = m_blockPositionMap.find(block_handle(i->second, 0)); // the size does not matter in this lookup

			block_handle free_block = j->first;

			m_blockSizeMap.erase(i);
			m_blockPositionMap.erase(j);

			// create the new block_handle and update the block handle for the free block
			block_handle res(free_block.position, size);
			free_block.size -= size;

			// insert the new free_block into the map and return the new block handle
			size_map_t::iterator k = m_blockSizeMap.insert(std::make_pair(free_block.size, free_block.position)).first;
			m_blockPositionMap.insert(std::make_pair(free_block, k));

			return res;
		}

		stream_size_type used_space() {
			position_map_t::iterator i = m_blockPositionMap.end(); --i;

			return i->first.position;
		}

		void set_configuration(block & configuration) {
			clear();

			block::iterator i = configuration.begin();
			block::iterator end = configuration.end();

			while(i != end) {
				stream_size_type pos = *i++;
				stream_size_type size = *i++;

				size_map_t::iterator j = m_blockSizeMap.insert(std::make_pair(size, pos)).first;
				m_blockPositionMap.insert(std::make_pair(block_handle(pos, size), j));
			}
		}

		void get_configuration(block & configuration) {
			configuration.resize(m_blockPositionMap.size() * sizeof(stream_size_type) * 2);

			block::iterator i = configuration.begin();
			for(position_map_t::iterator j = m_blockPositionMap.begin(), end = m_blockPositionMap.end(); j != end; ++j) {
				*i++ = j->first.position;
				*i++ = j->first.size;
			}

			tp_assert(i == configuration.end(), "configuration block was not resized correctly");
		}

		void clear() {
			m_blockPositionMap.clear();
			m_blockSizeMap.clear();
		}
	private:
		position_map_t m_blockPositionMap;
		size_map_t m_blockSizeMap;
	};
} // namespace bits

class block_collection {
public:
	block_collection()
	: m_open(false)
	, m_writeable(false)
	{}

	~block_collection() {
		close();
	}

	bool is_open() const {
		return m_open;
	}

	void open(std::string fileName, bool writeable) {
		close();

		if(writeable)
			m_accessor.open_wo(fileName);
		else
			m_accessor.open_ro(fileName);


		if(m_accessor.file_size_i() == 0) {
			m_collection.clear();
		}
		else {
			stream_size_type data_size;
			m_accessor.read_i(static_cast<void*>(&data_size), sizeof(stream_size_type));

			stream_size_type configuration_size = m_accessor.file_size_i() - sizeof(stream_size_type) - data_size;
			block configuration(configuration_size);

			m_accessor.seek_i(sizeof(stream_size_type) + data_size);
			m_accessor.read_i(static_cast<void*>(configuration.get()), configuration_size);
			m_collection.set_configuration(configuration);
		}

		m_writeable = writeable;
		m_open = true;
	}

	void close() {
		if(m_open) {
			stream_size_type data_size = m_collection.used_space();
			block configuration;
			m_collection.get_configuration(configuration);

			m_accessor.seek_i(0);
			m_accessor.write_i(static_cast<void*>(&data_size), sizeof(stream_size_type));

			m_accessor.seek_i(sizeof(stream_size_type) + data_size);
			m_accessor.write_i(static_cast<void*>(configuration.get()), configuration.size());
			m_accessor.close_i();

			m_open = false;
			m_collection.clear();
		}
	}

	block_handle get_free_block(stream_size_type size) {
		tp_assert(m_writeable, "get_free_block(): the block collection is read only");

		return m_collection.alloc(size);
	}

	void free_block(block_handle handle) {
		tp_assert(m_writeable, "free_block(): the block collection is read only");

		m_collection.free(handle);
	}

	void read_block(block_handle handle, block & b) {
		b.resize(handle.size);

		m_accessor.seek_i(handle.position + sizeof(stream_size_type));
		m_accessor.read_i(static_cast<void*>(b.get()), handle.size);
	}

	void write_block(block_handle handle, const block & b) {
		tp_assert(m_writeable, "write_block(): the block collection is read only");

		m_accessor.seek_i(handle.position + sizeof(stream_size_type));
		m_accessor.write_i(static_cast<const void*>(b.get()), handle.size);
	}
private:
	bits::freespace_collection m_collection;
	tpie::file_accessor::raw_file_accessor m_accessor;

	bool m_open;
	bool m_writeable;
};

} // blocks namespace

}  //  tpie namespace

#endif // _TPIE_BLOCK_COLLECTION_H
