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
/// \file block_collection_cache Class to handle caching of the block collection
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_BLOCKS_BLOCK_COLLECTION_CACHE_H
#define _TPIE_BLOCKS_BLOCK_COLLECTION_CACHE_H

#include <tpie/tpie.h>
#include <tpie/tpie_assert.h>
#include <tpie/file_accessor/file_accessor.h>
#include <tpie/blocks/block.h>
#include <tpie/blocks/block_collection.h>
#include <list>
#include <map>

namespace tpie {

namespace blocks {

class block_collection_cache {
private:
	struct position_comparator {
		bool operator()(const block_handle & a, const block_handle & b) {
			return a.position < b.position;
		}
	};

	typedef std::list<block_handle> block_list_t;
	typedef std::map<block_handle, std::pair<block*, block_list_t::iterator>, position_comparator> block_map_t;
public:
	block_collection_cache()
	: m_collection()
	, m_curSize(0)
	{}

	block_collection_cache(std::string fileName, bool writeable, memory_size_type maxSize)
	: m_curSize(0)
	{
		open(fileName, writeable, maxSize);
	}

	~block_collection_cache() {
		close();
	}

	bool is_open() const {
		return m_collection.is_open();
	}

	void open(std::string fileName, bool writeable, memory_size_type maxSize) {
		m_maxSize = maxSize;
		m_collection.open(fileName, writeable);
	}

	void close() {

		if(m_collection.is_open()) {
			// write the content of the cache to disk
			block_map_t::iterator end = m_blockMap.end();

			for(block_map_t::iterator i = m_blockMap.begin(); i != end; ++i) {
				m_collection.write_block(i->first, *i->second.first);
				tpie_delete(i->second.first);
			}

			// set to the initial state
			m_curSize = 0;
			m_blockList.clear();
			m_blockMap.clear();

			m_collection.close();
		}
	}

	block_handle get_free_block(stream_size_type size) {
		block_handle h = m_collection.get_free_block(size);
		return h;
	}

	void free_block(block_handle handle) {
		block_map_t::iterator i = m_blockMap.find(handle);

		if(i != m_blockMap.end()) {
			m_blockList.erase(i->second.second);
			tpie_delete(i->second.first);
			m_curSize -= i->first.size;
			m_blockMap.erase(i);
		}

		m_collection.free_block(handle);
	}

private:
	void dump_cache(memory_size_type size) {
		while(m_curSize > m_maxSize - size) { // while there isn't space in the cache
			// write the last accessed block to disk
			block_handle handle = m_blockList.front();
			m_blockList.pop_front();

			block_map_t::iterator i = m_blockMap.find(handle);
			m_collection.write_block(i->first, *(i->second.first));
			tpie_delete(i->second.first);
			m_curSize -= i->first.size;
			m_blockMap.erase(i);
		}
	}

	void add_to_cache(block_handle handle, block * b) {
		m_blockList.push_back(handle);
		block_list_t::iterator list_pos = m_blockList.end();
		--list_pos;

		m_blockMap[handle] = std::make_pair(b, list_pos);
		m_curSize += handle.size;
	}
public:
	void read_block(block_handle handle, block & b) {
		block_map_t::iterator i = m_blockMap.find(handle);

		if(i != m_blockMap.end()) { // the block is already in the cache
			// update the block list to reflect that the block was accessed
			m_blockList.erase(i->second.second);
			m_blockList.push_back(i->first);

			// return the block content from cache
			b = *i->second.first;
			return;
		}

		// the block isn't in the cache
		dump_cache(handle.size); // make space in the cache

		block * cache_b = tpie_new<block>();
		m_collection.read_block(handle, *cache_b);
		b = *cache_b;

		add_to_cache(handle, cache_b);
	}

	void write_block(block_handle handle, const block & b) {
		block_map_t::iterator i = m_blockMap.find(handle);

		if(i != m_blockMap.end()) {
			block * cache_b = i->second.first;
			*cache_b = b;

			m_blockList.erase(i->second.second);
			m_blockList.push_back(handle);

			block_list_t::iterator j = m_blockList.end();
			--j;

			*(i->second.first) = b;
			i->second.second = j;
			return;
		}

		dump_cache(handle.size);

		block * cache_b = tpie_new<block>();
		*cache_b = b;

		add_to_cache(handle, cache_b);
	}
private:
	block_collection m_collection;
	block_list_t m_blockList;
	block_map_t m_blockMap;
	memory_size_type m_curSize;
	memory_size_type m_maxSize;
};

} // blocks namespace

}  //  tpie namespace

#endif // _TPIE_BLOCKS_BLOCK_COLLECTION_CACHE_H
