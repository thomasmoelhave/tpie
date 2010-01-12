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

#include <tpie/file.h>
#include <tpie/exception.h>

namespace tpie {

// template <typename BTE>
// fd_file_base<BTE>::fd_file_base(size_type bs, size_type is, bool cr, bool cw, boost::uint64_t typeMagic):
// 	blockItems(bs/is), m_size(0), canRead(cr), canWrite(cw), itemSize(is),
// 	firstUsed(0), firstFree(0), bte(canRead, canWrite, itemSize, typeMagic) {

// 	emptyBlock.size = 0;
// 	emptyBlock.number = std::numeric_limits<offset_type>::max();
// 	emptyBlock.next = 0;
// }

file_base::file_base(memory_size_type itemSize, 
					 float blockFactor, 
					 file_accessor::file_accessor * fileAccessor) :
	m_size(0), 	m_itemSize(itemSize),  m_firstUsed(0), m_firstFree(0)
{
	if (fileAccessor == 0)
		fileAccessor = new default_file_accessor();
	m_fileAccessor = fileAccessor;
	m_emptyBlock.size = 0;
	m_emptyBlock.number = std::numeric_limits<stream_size_type>::max();
	m_emptyBlock.next = 0;

	m_blockItems = 2*1024*1024/m_itemSize;
}
	

void file_base::create_block() {
	block_t * block = reinterpret_cast<block_t*>( new char[sizeof(block_t) + m_itemSize*m_blockItems] );
	block->next = m_firstFree;
	m_firstFree = block;
}

void file_base::delete_block() {
	block_t * block = m_firstFree;
	assert(block != 0);
	m_firstFree = block->next;
	delete[] reinterpret_cast<char*>(block);
}

file_base::block_t * file_base::get_block(stream_size_type block) {
	block_t * b = m_firstUsed;
	while(b && b->number != block)
		b = b->next;
	if (b == 0) {
		b = m_firstFree;
		m_firstFree = b->next;
		b->dirty = false;
		b->next = m_firstUsed;
		m_firstUsed = b;
		b->number = block;
		b->usage = 0;
		
		b->size = m_blockItems;
		if (static_cast<stream_size_type>(b->size) + b->number * static_cast<stream_size_type>(m_blockItems) > size())
			b->size = size() - b->number * m_blockItems;

		if (b->size > 0 &&
			m_fileAccessor->read(b->data, b->number * static_cast<stream_size_type>(m_blockItems), b->size) != b->size) 
			throw io_exception("Incorrect number of items read");
	}
	++b->usage;
	return b;
}

void file_base::free_block(block_t * block) {
	--block->usage;
	if (block->usage > 0) return;

	if (block->dirty || !m_canRead) {
		assert(m_canWrite);
		m_fileAccessor->write(block->data, block->number * static_cast<stream_size_type>(m_blockItems), block->size);
	}
	
	block_t * prev=m_firstUsed;
	while(prev && prev->next != block) prev = prev->next;
	if (prev == 0) {
		assert(m_firstUsed == block);
		m_firstUsed = block->next;
	} else
		prev->next = block->next;
	
	block->next = m_firstFree;
	m_firstFree = block;
}

file_base::~file_base() {
	assert(m_firstFree == 0);
	assert(m_firstUsed == 0);
	delete m_fileAccessor;
}

void file_base::stream::update_block() {
	update_vars();
	if (m_nextBlock == std::numeric_limits<memory_size_type>::max()) {
		m_nextBlock = m_block->number+1;
		m_nextIndex = 0;
	}
	if (m_block != &m_file.m_emptyBlock) m_file.free_block(m_block);
	m_block = m_file.get_block(m_nextBlock);
	m_index = m_nextIndex;
	m_nextBlock = std::numeric_limits<stream_size_type>::max();
	m_nextIndex = std::numeric_limits<memory_size_type>::max();
}

file_base::stream::stream(file_base & f, stream_size_type offset):
	m_file(f) {
	m_nextBlock = std::numeric_limits<stream_size_type>::max();
	m_nextIndex = std::numeric_limits<memory_size_type>::max();
	m_index = std::numeric_limits<memory_size_type>::max();;
	m_block = &m_file.m_emptyBlock;
	m_file.create_block();
	seek(offset);
}

void file_base::stream::free() {
	if (m_block) {
		update_vars();
		if (m_block != &m_file.m_emptyBlock) m_file.free_block(m_block);
		m_file.delete_block();
	}
	m_block = 0;
}


}

