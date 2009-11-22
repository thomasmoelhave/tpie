// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, The TPIE development team
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

#ifndef __TPIE_STREAM_FD_FILE_BASE_INL_
#define __TPIE_STREAM_FD_FILE_BASE_INL_
#include <tpie/stream/fd_file.h>
#include <tpie/stream/exception.h>

namespace tpie {
namespace stream {

template <typename BTE>
fd_file_base<BTE>::fd_file_base(size_type bs, size_type is, bool cr, bool cw, uint64_t typeMagic):
	blockItems(bs/is), m_size(0), canRead(cr), canWrite(cw), itemSize(is),
	firstUsed(0), firstFree(0), bte(canRead, canWrite, itemSize, typeMagic) {

	emptyBlock.size = 0;
	emptyBlock.number = std::numeric_limits<offset_type>::max();
	emptyBlock.next = 0;
}

template <typename BTE>
void fd_file_base<BTE>::create_block() {
	block_t * block = reinterpret_cast<block_t*>( new char[sizeof(block_t) + itemSize*blockItems] );
	block->next = firstFree;
	firstFree = block;
}

template <typename BTE>
void fd_file_base<BTE>::delete_block() {
	block_t * block = firstFree;
	assert(block != 0);
	firstFree = block->next;
	delete[] reinterpret_cast<char*>(block);
}

template <typename BTE>
typename fd_file_base<BTE>::block_t * fd_file_base<BTE>::get_block(offset_type block) {
	block_t * b = firstUsed;
	while(b && b->number != block)
		b = b->next;
	if (b == 0) {
		b = firstFree;
		firstFree = b->next;
		b->dirty = false;
		b->next = firstUsed;
		firstUsed = b;
		b->number = block;
		b->usage = 0;
		
		b->size = blockItems;
		if ((offset_type)b->size + b->number * (offset_type)blockItems > size())
			b->size = size() - b->number * blockItems;

		if (canRead && b->size > 0) {
			if (bte.read(b->data, b->number * blockItems, b->size) != b->size) 
				throw io_exception("Incorrect number of items read");
		}
	}
	++b->usage;
	return b;
}

template <typename BTE>
void fd_file_base<BTE>::free_block(block_t * block) {
	--block->usage;
	if (block->usage > 0) return;

	if (block->dirty || !canRead) {
		assert(canWrite);
		bte.write(block->data, block->number * blockItems, block->size);
	}
	
	block_t * prev=firstUsed;
	while(prev && prev->next != block) prev = prev->next;
	if (prev == NULL) {
		assert(firstUsed == block);
		firstUsed = block->next;
	} else
		prev->next = block->next;
	
	block->next = firstFree;
	firstFree = block;
}

template <typename BTE>
fd_file_base<BTE>::~fd_file_base() {
	assert(firstFree == 0);
	assert(firstUsed == 0);
}

template <typename BTE>
void fd_file_base<BTE>::stream::update_vars() {
	if (index != std::numeric_limits<size_type>::max()) 
		block->size = std::max(block->size, index);
	if (index != std::numeric_limits<size_type>::max() &&
		block->number != std::numeric_limits<offset_type>::max())
		file.m_size = std::max(file.m_size, block->number * (offset_type)file.blockItems + (offset_type)index);
}

template <typename BTE>
void fd_file_base<BTE>::stream::update_block() {
	update_vars();
	if (nextBlock == std::numeric_limits<offset_type>::max()) {
		nextBlock = block->number+1;
		nextIndex = 0;
	}
	if (block != &file.emptyBlock) file.free_block(block);
	block = file.get_block(nextBlock);
	index = nextIndex;
	nextBlock = std::numeric_limits<offset_type>::max();
	nextIndex = std::numeric_limits<size_type>::max();
}

template <typename BTE>
void fd_file_base<BTE>::stream::seek(offset_type offset) {
	update_vars();
	offset_type b = offset / file.blockItems;
	index = offset - b*file.blockItems;
	if (b == block->number) {
		nextBlock = std::numeric_limits<offset_type>::max();
		nextIndex = std::numeric_limits<size_type>::max();
		return;
	}
	nextBlock = b;
	nextIndex = index;
	index = std::numeric_limits<size_type>::max();
}

template <typename BTE>
fd_file_base<BTE>::stream::stream(fd_file_base & f, offset_type o):
	file(f) {
	nextBlock = std::numeric_limits<offset_type>::max();
	nextIndex = std::numeric_limits<size_type>::max();
	index = std::numeric_limits<size_type>::max();;
	block = &file.emptyBlock;
	file.create_block();
	seek(o);
}

template <typename BTE>
fd_file_base<BTE>::stream::~stream() {
	update_vars();
	if (block != &file.emptyBlock) file.free_block(block);
	file.delete_block();
}


}
}
#endif //__TPIE_STREAM_FD_FILE_BASE_INL_
