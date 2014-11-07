// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
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
#ifndef __BLOCK_COLLECTION_H__
#define __BLOCK_COLLECTION_H__

#include "common.h"
#include <tpie/tpie.h>
#include <tpie/blocks/block_collection.h>
#include <tpie/blocks/block_collection_cache.h>
#include <tpie/tempname.h>
#include <vector>
#include <deque>
#include <algorithm>
#include <tpie/file_accessor/file_accessor.h>

using namespace tpie;
using namespace tpie::blocks;

const stream_size_type max_block_size = 1024 * 1024;

memory_size_type random(memory_size_type i) {
	return 179424673 * i + 15485863;
}

template<typename T>
bool basic(T & collection) {
	std::vector<block_handle> blocks;

	// write 20 twenty blocks of random sizes
	for(char i = 0; i < 20; ++i) {
		stream_size_type size = random(i) % max_block_size + 1;

		block_handle handle = collection.get_free_block(size);

		TEST_ENSURE(handle.size >= size, "The returned block size is too small.");

		block b(handle.size);

		for(block::iterator j = b.begin(); j != b.end(); ++j)
			*j = i;

		collection.write_block(handle, b);
		blocks.push_back(handle);
	}

	log_debug() << "Finished writing blocks." << std::endl;

	// verify the content of the 20 blocks
	for(char i = 0; i < 20; ++i) {
		block_handle handle = blocks[i];

		block b;
		collection.read_block(handle, b);

		TEST_ENSURE_EQUALITY(handle.size, b.size(), "The block size should be equal to the handle size");

		for(block::iterator j = b.begin(); j != b.end(); ++j)
			TEST_ENSURE_EQUALITY((int) *j, (int) i, "the content of the returned block is not correct");
	}

	log_debug() << "Finished reading blocks." << std::endl;

	collection.close();

	log_debug() << "Closed collection." << std::endl;
	return true;
}

template<typename T>
bool erase(T & collection) {
	typedef std::list<std::pair<block_handle, char> > block_list_t;
	block_list_t blocks;

	// write 20 twenty blocks of random sizes
	for(char i = 0; i < 20; ++i) {
		stream_size_type size = random(i) % max_block_size + 1;

		block_handle handle = collection.get_free_block(size);

		TEST_ENSURE(handle.size >= size, "The returned block size is too small.");

		block b(handle.size);

		for(block::iterator j = b.begin(); j != b.end(); ++j)
			*j = i;

		collection.write_block(handle, b);
		blocks.push_back(std::make_pair(handle, i));
	}

	// repeat 40 times: free a block then allocate a block
	for(char i = 20; i < 40; ++i) {
		// free a block
		block_list_t::iterator j = blocks.begin();
		std::advance(j, random(i) % blocks.size());
		collection.free_block(j->first);
		blocks.erase(j);

		// allocate a new block

		stream_size_type size = random(i) % max_block_size + 1;

		block_handle handle = collection.get_free_block(size);

		TEST_ENSURE(handle.size >= size, "The returned block size is too small.");

		block b(handle.size);

		for(block::iterator j = b.begin(); j != b.end(); ++j)
			*j = i;

		collection.write_block(handle, b);
		blocks.push_back(std::make_pair(handle, i));
	}

	// verify the content of the 20 blocks
	for(block_list_t::iterator i = blocks.begin(); i != blocks.end(); ++i) {
		block_handle handle = i->first;
		char content = i->second;

		block b;
		collection.read_block(handle, b);

		TEST_ENSURE_EQUALITY(handle.size, b.size(), "The block size should be equal to the handle size");

		for(block::iterator j = b.begin(); j != b.end(); ++j)
			TEST_ENSURE_EQUALITY((int) *j, (int) content, "the content of the returned block is not correct"); // cast to int for human-readable human
	}

	collection.close();

	return true;
}

template<typename T>
bool overwrite(T & collection) {
	typedef std::list<std::pair<block_handle, char> > block_list_t;
	block_list_t blocks;

	// write 20 twenty blocks of random sizes
	for(char i = 0; i < 20; ++i) {
		stream_size_type size = random(i) % max_block_size + 1;

		block_handle handle = collection.get_free_block(size);

		TEST_ENSURE(handle.size >= size, "The returned block size is too small.");

		block b(handle.size);

		for(block::iterator j = b.begin(); j != b.end(); ++j)
			*j = i;

		collection.write_block(handle, b);
		blocks.push_back(std::make_pair(handle, i));
	}

	// repeat 20 times: overwrite a random block
	for(char i = 20; i < 40; ++i) {
		// select a random block handle from the list
		block_list_t::iterator j = blocks.begin();
		std::advance(j, random(i) % blocks.size());
		block_handle h = j->first;

		// overwrite the contents of the block
		block b(h.size);
		for(block::iterator j = b.begin(); j != b.end(); ++j)
			*j = i;
		collection.write_block(h, b);

		// update the block list
		blocks.erase(j);
		blocks.push_back(std::make_pair(h, i));
	}

	// verify the content of the 20 blocks
	for(block_list_t::iterator i = blocks.begin(); i != blocks.end(); ++i) {
		block_handle handle = i->first;
		char content = i->second;

		block b;
		collection.read_block(handle, b);

		TEST_ENSURE_EQUALITY(handle.size, b.size(), "The block size should be equal to the handle size");

		for(block::iterator j = b.begin(); j != b.end(); ++j)
			TEST_ENSURE_EQUALITY((int) *j, (int) content, "the content of the returned block is not correct"); // cast to int for human-readable human
	}

	collection.close();

	return true;
}


#endif //__BLOCK_COLLECTION_H__
