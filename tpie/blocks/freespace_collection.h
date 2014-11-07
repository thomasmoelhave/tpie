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

#ifndef _TPIE_BLOCKS_FREESPACE_COLLECTION_H
#define _TPIE_BLOCKS_FREESPACE_COLLECTION_H

#include <tpie/tpie.h>
#include <tpie/tpie_assert.h>
#include <tpie/array.h>
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

	typedef std::multimap<stream_size_type, stream_size_type> size_map_t;
	typedef std::map<block_handle, size_map_t::iterator, position_comparator> position_map_t;
public:
	freespace_collection() {
		// the initial configuration is an empty block with the maximum possible size
		initial_configuration();
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
		tp_assert(m_blockSizeMap.size() > 0, "the block_size_map should contain at least one chunk.");
		size_map_t::iterator i = m_blockSizeMap.lower_bound(size);
		tp_assert(i != m_blockSizeMap.end(), "The freespace_collection ran out of space.");

		position_map_t::iterator j = m_blockPositionMap.find(block_handle(i->second, 0)); // the size does not matter in this lookup

		block_handle free_block = j->first;

		m_blockSizeMap.erase(i);

		m_blockPositionMap.erase(j);

		// create the new block_handle and update the block handle for the free block
		block_handle res(free_block.position, size);
		free_block.size -= size;
		free_block.position += size;

		// insert the new free_block into the map and return the new block handle
		if(free_block.size > 0) {
			size_map_t::iterator k = m_blockSizeMap.insert(std::make_pair(free_block.size, free_block.position)).first;
			m_blockPositionMap.insert(std::make_pair(free_block, k));
		}

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

		stream_size_type * i = reinterpret_cast<stream_size_type *>(configuration.get());
		for(position_map_t::iterator j = m_blockPositionMap.begin(), end = m_blockPositionMap.end(); j != end; ++j) {
			*i++ = j->first.position;
			*i++ = j->first.size;
		}
	}

	void initial_configuration() {
		clear();
		stream_size_type size = std::numeric_limits<stream_size_type>::max();
		stream_size_type pos = 0;

		size_map_t::iterator i = m_blockSizeMap.insert(std::make_pair(size, pos)).first;
		m_blockPositionMap.insert(std::make_pair(block_handle(pos, size), i));
	}

private:
	void clear() {
		m_blockPositionMap.clear();
		m_blockSizeMap.clear();
	}

	position_map_t m_blockPositionMap;
	size_map_t m_blockSizeMap;
};

} // bits namespace

} // blocks namespace

} // tpie namespace

#endif // _TPIE_BLOCKS_FREESPACE_COLLECTION_H
