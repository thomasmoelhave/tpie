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
		bool operator()(block_handle a, block_handle b) {
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

private:
	// merge the handle with free block before it ( if any )
	block_handle merge_with_block_before(block_handle handle) {
		if(m_blockPositionMap.empty())
			return handle;

		position_map_t::iterator i = m_blockPositionMap.upper_bound(handle); // find the first block whose position is greater than that of the block handle
		if(i == m_blockPositionMap.begin())
			return handle;

		--i;
		block_handle prev = i->first;

		if(prev.position + prev.size != handle.position)
			return handle;

		// merge the two blocks if they are adjacent
		handle.position = prev.position;
		handle.size += prev.size;

		tp_assert(i->first.position == i->second->second, "Inconsistent position between the two maps");
		tp_assert(i->first.size == i->second->first, "Inconsistent size between the two maps");

		m_blockSizeMap.erase(i->second);
		m_blockPositionMap.erase(i);
		return handle;
	}

	// merge the handle with free block after it ( if any )
	block_handle merge_with_block_after(block_handle handle) {
		position_map_t::iterator i = m_blockPositionMap.upper_bound(handle); // find the first block whose position is greater than that of the block handle
		if(i == m_blockPositionMap.end())
			return handle;
		block_handle next = i->first;

		if(handle.position + handle.size == next.position) { // merge the two blocks if they are adjacent
			handle.size += next.size;
			m_blockSizeMap.erase(i->second);
			m_blockPositionMap.erase(i);
		}

		return handle;
	}

	void insert_free_block(block_handle handle) {
		size_map_t::iterator i = m_blockSizeMap.insert(std::make_pair(handle.size, handle.position));
		position_map_t::iterator j = m_blockPositionMap.insert(std::make_pair(handle, i)).first; // update the block map to contain the now free block

		tp_assert(i->first == j->first.size, "Inconsistent size insertion.");
		tp_assert(i->second == j->first.position, "Inconsistent position insertion.");
	}
public:
	void free(block_handle handle) {
		handle = merge_with_block_before(handle);
		handle = merge_with_block_after(handle);

		insert_free_block(handle);
	}

private:
	// ceil i to nearest multiple of mul
	stream_size_type mult_ceil(stream_size_type i, stream_size_type mul) {
		stream_size_type mod = i % mul;
		if(mod != 0)
			i += mul - mod;
		return i;
	}

	// remove a block from the free map larger than size
	block_handle take_free_block(stream_size_type size) {
		tp_assert(m_blockSizeMap.size() > 0, "the block_size_map should contain at least one chunk.");
		size_map_t::iterator i = m_blockSizeMap.lower_bound(size);
		tp_assert(i != m_blockSizeMap.end(), "The freespace_collection ran out of space.");

		position_map_t::iterator j = m_blockPositionMap.find(block_handle(i->second, i->first)); // the size is not used during this lookup
		i = j->second; // in case of multiple blocks with same size

		block_handle free_block = j->first;

		m_blockSizeMap.erase(i);
		m_blockPositionMap.erase(j);

		return free_block;
	}
public:
	block_handle alloc(stream_size_type size) {
		tp_assert(size > 0, "The block size should be greather than zero");

		size = mult_ceil(size, alignment);

		block_handle free_block = take_free_block(size);
		block_handle res(free_block.position, size);
		free_block.size -= size;
		free_block.position += size;

		if(free_block.size > 0)
			insert_free_block(free_block);

		return res;
	}

	stream_size_type used_space() const {
		position_map_t::const_iterator i = m_blockPositionMap.end(); --i;

		return i->first.position;
	}

	void set_configuration(block & configuration) {
		clear();

		block::iterator i = configuration.begin();
		block::iterator end = configuration.end();

		while(i != end) {
			stream_size_type pos = *i++;
			stream_size_type size = *i++;

			size_map_t::iterator j = m_blockSizeMap.insert(std::make_pair(size, pos));
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

		size_map_t::iterator i = m_blockSizeMap.insert(std::make_pair(size, pos));
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
