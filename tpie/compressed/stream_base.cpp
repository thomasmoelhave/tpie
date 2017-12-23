// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2013, The TPIE development team
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

#include <tpie/compressed/stream.h>
#include <tpie/file_base_crtp.inl>

namespace tpie {

compressed_stream_base::compressed_stream_base(memory_size_type itemSize,
											   double blockFactor)
	: m_bufferDirty(false)
	, m_blockItems(block_size(blockFactor) / itemSize)
	, m_blockSize(block_size(blockFactor))
	, m_canRead(false)
	, m_canWrite(false)
	, m_open(false)
	, m_itemSize(itemSize)
	, m_cachedReads(0)
	, m_cachedWrites(0)
	, m_ownedTempFile(/* empty unique_ptr */)
	, m_tempFile(0)
	, m_byteStreamAccessor()
	, m_size(0)
	, m_buffers(m_blockSize)
	, m_buffer(/* empty shared_ptr */)
	, m_streamBlocks(0)
	, m_lastBlockReadOffset(0)
	, m_currentFileSize(0)
	, m_response()
	, m_seekState(seek_state::beginning)
	, m_readOffset(0)
	, m_offset(0)
	, m_nextPosition(/* not a position */)
	, m_nextReadOffset(0)
	, m_bufferBegin(nullptr)
	, m_bufferEnd(nullptr)
	, m_nextItem(nullptr)
{
	// Empty constructor.
}

compressed_stream_base::~compressed_stream_base() {
	// We cannot close() here since flush_block() is pure virtual at this point.
	if (is_open())
		log_debug() << "compressed_stream_base destructor reached "
			<< "while stream is still open." << std::endl;
	// non-trivial field destructors:
	// m_response::~compressor_response()
	// m_buffer::~shared_ptr()
	// m_buffers::~stream_buffers()
	// m_byteStreamAccessor::~byte_stream_accessor()
	// m_ownedTempFile::~unique_ptr()
}

void compressed_stream_base::open_inner(const std::string & path,
										open::type openFlags,
										memory_size_type userDataSize)
{
	// Parse openFlags
	const bool readOnly = openFlags & open::read_only;
	const bool writeOnly = openFlags & open::write_only;
	if (readOnly && writeOnly)
		throw tpie::stream_exception("Invalid read/write only flags");
	m_canRead = !writeOnly;
	m_canWrite = !readOnly;

	const cache_hint cacheHint = open::translate_cache(openFlags);
	const compression_flags compressionFlags = open::translate_compression(openFlags);

	m_byteStreamAccessor.open(path, m_canRead, m_canWrite, m_itemSize,
							  m_blockSize, userDataSize, cacheHint,
							  compressionFlags);
	m_size = m_byteStreamAccessor.size();
	m_open = true;
	m_streamBlocks = (m_size + m_blockItems - 1) / m_blockItems;
	m_lastBlockReadOffset = m_byteStreamAccessor.get_last_block_read_offset();
	m_currentFileSize = m_byteStreamAccessor.file_size();
	m_response.clear_block_info();

	this->post_open();
}

/*static*/ memory_size_type compressed_stream_base::block_size(double blockFactor) throw () {
	return static_cast<memory_size_type>(get_block_size() * blockFactor);
}

/*static*/ double compressed_stream_base::calculate_block_factor(memory_size_type blockSize) throw () {
	return (double)blockSize / (double)block_size(1.0);
}

/*static*/ memory_size_type compressed_stream_base::block_memory_usage(double blockFactor) {
	return block_size(blockFactor);
}

memory_size_type compressed_stream_base::block_items() const {
	return m_blockItems;
}

memory_size_type compressed_stream_base::block_size() const {
	return m_blockSize;
}

memory_size_type compressed_stream_base::read_user_data(void * data, memory_size_type count) {
	tp_assert(is_open(), "read_user_data: !is_open");
	return m_byteStreamAccessor.read_user_data(data, count);
}

void compressed_stream_base::write_user_data(const void * data, memory_size_type count) {
	tp_assert(is_open(), "write_user_data: !is_open");
	m_byteStreamAccessor.write_user_data(data, count);
}

memory_size_type compressed_stream_base::user_data_size() const {
	tp_assert(is_open(), "user_data_size: !is_open");
	return m_byteStreamAccessor.user_data_size();
}

memory_size_type compressed_stream_base::max_user_data_size() const {
	tp_assert(is_open(), "max_user_data_size: !is_open");
	return m_byteStreamAccessor.max_user_data_size();
}

const std::string & compressed_stream_base::path() const {
	assert(m_open);
	return m_byteStreamAccessor.path();
}

void compressed_stream_base::open(const std::string & path, open::type openFlags,
								  memory_size_type userDataSize /*= 0*/)
{
	close();
	open_inner(path, openFlags, userDataSize);
}

void compressed_stream_base::open(open::type openFlags,
								  memory_size_type userDataSize /*= 0*/)
{
	close();
	m_ownedTempFile.reset(tpie_new<temp_file>());
	m_tempFile = m_ownedTempFile.get();
	open_inner(m_tempFile->path(), openFlags, userDataSize);
}

void compressed_stream_base::open(temp_file & file, open::type openFlags,
								  memory_size_type userDataSize /*= 0*/)
{
	close();
	m_tempFile = &file;
	open_inner(m_tempFile->path(), openFlags, userDataSize);
}

void compressed_stream_base::close() {
	uncache_read_writes();
	if (m_open) {
		compressor_thread_lock l(compressor());

		if (m_bufferDirty)
			flush_block(l);

		m_buffer.reset();

		finish_requests(l);

		if (use_compression()) {
			m_byteStreamAccessor.set_last_block_read_offset(last_block_read_offset(l));
		}
		m_byteStreamAccessor.set_size(m_size);
		m_byteStreamAccessor.close();
	}
	m_open = false;
	m_tempFile = NULL;
	m_ownedTempFile.reset();

	// If read/write is called on the closed stream,
	// perform_seek throws an exception.
	m_seekState = seek_state::beginning;
}

void compressed_stream_base::finish_requests(compressor_thread_lock & l) {
	tp_assert(!(m_buffer.get() != 0), "finish_requests called when own buffer is still held");
	m_buffers.clean();
	while (!m_buffers.empty()) {
		compressor().wait_for_request_done(l);
		m_buffers.clean();
	}
}

stream_size_type compressed_stream_base::last_block_read_offset(compressor_thread_lock & l) {
	tp_assert(use_compression(), "last_block_read_offset: !use_compression");
	if (m_streamBlocks == 0 || m_streamBlocks == 1)
		return 0;
	if (m_lastBlockReadOffset != std::numeric_limits<stream_size_type>::max())
		return m_lastBlockReadOffset;
	// We assume that streamBlocks is monotonically increasing over time;
	// the response object might throw otherwise.
	while (!m_response.has_block_info(m_streamBlocks - 1))
		m_response.wait(l);
	return m_response.get_read_offset(m_streamBlocks - 1);
}

stream_size_type compressed_stream_base::current_file_size(compressor_thread_lock & l) {
	tp_assert(use_compression(), "current_file_size: !use_compression");
	if (m_streamBlocks == 0)
		return 0;
	if (m_currentFileSize != std::numeric_limits<stream_size_type>::max())
		return m_currentFileSize;
	// We assume that streamBlocks is monotonically increasing over time;
	// the response object might throw otherwise.
	while (!m_response.has_block_info(m_streamBlocks - 1))
		m_response.wait(l);
	return m_response.get_read_offset(m_streamBlocks - 1)
		+ m_response.get_block_size(m_streamBlocks - 1);
}


void compressed_stream_base::truncate_zero() {
	// No need to flush block
	m_buffer.reset();
	m_response.clear_block_info();
	m_updateReadOffsetFromWrite = false;
	compressor_thread_lock l(compressor());
	finish_requests(l);
	get_buffer(l, 0);
	m_size = 0;
	m_streamBlocks = 0;
	m_byteStreamAccessor.truncate(0);
	
	m_readOffset = 0;
	m_offset = 0;
	m_nextItem = m_bufferBegin;
	m_seekState = seek_state::none;
	uncache_read_writes();
}

void compressed_stream_base::truncate_uncompressed(stream_size_type offset) {
	tp_assert(!use_compression(), "truncate_uncompressed called on compressed stream");
	
	stream_size_type currentOffset = this->offset();
	if (m_buffer.get() != 0
		&& block_number(offset) == buffer_block_number()
		&& buffer_block_number() == m_streamBlocks)
	{
		// We are truncating a final block that has not been written yet.
		m_size = offset;
		if (offset < m_offset) {
			m_offset = offset;
			memory_size_type blockItemIndex =
				static_cast<memory_size_type>(offset - m_streamBlocks * m_blockItems);
			m_nextItem = m_bufferBegin + m_itemSize * blockItemIndex;
		}
		m_bufferDirty = true;
		m_seekState = seek_state::none;
		// No need to update m_streamBlocks
	} else {
		// We need to do a truncate on the file accessor.
		// Get rid of the current block first.
		compressor_thread_lock l(compressor());
		if (offset < buffer_block_number() * m_blockItems) {
			// No need to flush current block, since we are truncating it away.
		} else {
			// Changes to the current block may still be visible after the truncate.
			if (m_bufferDirty) {
				m_updateReadOffsetFromWrite = false;
				flush_block(l);
			}
		}
		m_buffer.reset();
		m_bufferDirty = false;
		finish_requests(l);
		m_byteStreamAccessor.truncate(offset);
		m_size = offset;
		m_streamBlocks = (offset + m_blockItems - 1) / m_blockItems;
	}
	seek(std::min(currentOffset, offset));
}

void compressed_stream_base::truncate_compressed(const stream_position & pos) {
	tp_assert(use_compression(), "truncate_compressed called on uncompressed stream");
	
	stream_size_type offset = pos.offset();
	stream_position finalDestination = (offset < this->offset()) ? pos : get_position();
	
	if (m_buffer.get() == 0 || block_number(offset) != buffer_block_number()) {
		set_position(pos);
		perform_seek();
	}
	
	// We are truncating into the currently loaded block.
	if (buffer_block_number() < m_streamBlocks) {
		m_streamBlocks = buffer_block_number() + 1;
		m_lastBlockReadOffset = pos.read_offset();
		m_currentFileSize = std::numeric_limits<stream_size_type>::max();
		compressor_thread_lock l(compressor());
		m_response.clear_block_info();
		m_updateReadOffsetFromWrite = false;
	}
	m_size = offset;
	if (offset < m_offset) {
		m_offset = offset;
		memory_size_type blockItemIndex =
			static_cast<memory_size_type>(offset - m_streamBlocks * m_blockItems);
		m_nextItem = m_bufferBegin + m_itemSize * blockItemIndex;
	}
	m_bufferDirty = true;
	
	set_position(finalDestination);
}

void compressed_stream_base::get_buffer(compressor_thread_lock & l, stream_size_type blockNumber) {
	uncache_read_writes();
	buffer_t().swap(m_buffer);
	m_buffer = this->m_buffers.get_buffer(l, blockNumber);
	while (m_buffer->is_busy()) compressor().wait_for_request_done(l);
	m_bufferBegin = m_buffer->get();
	m_bufferEnd = m_bufferBegin + m_itemSize * block_items();
	this->m_bufferDirty = false;
}

void compressed_stream_base::flush_block(compressor_thread_lock & lock) {
	uncache_read_writes();
	stream_size_type blockNumber = buffer_block_number();
	stream_size_type writeOffset;
	if (!use_compression()) {
		// Uncompressed case
		writeOffset = blockNumber * m_blockSize;
	} else {
		// Compressed case
		if (blockNumber == m_streamBlocks) {
			// New block; no truncate
			writeOffset = std::numeric_limits<stream_size_type>::max();
		} else if (blockNumber == m_streamBlocks - 1) {
			// Block rewrite; truncate
			writeOffset = last_block_read_offset(lock);
			m_response.clear_block_info();
		} else {
			throw exception("flush_block: blockNumber not at end of stream");
		}
	}
	
	m_lastBlockReadOffset = std::numeric_limits<stream_size_type>::max();
	m_currentFileSize = std::numeric_limits<stream_size_type>::max();
	
	if (m_nextItem == nullptr) throw exception("m_nextItem is NULL");
	if (m_bufferBegin == nullptr) throw exception("m_bufferBegin is NULL");
	memory_size_type blockItems = m_blockItems;
	if (blockItems + blockNumber * m_blockItems > size()) {
		blockItems =
			static_cast<memory_size_type>(size() - blockNumber * m_blockItems);
	}
	m_buffer->set_size(blockItems * m_itemSize);
	m_buffer->set_state(compressor_buffer_state::writing);
	compressor_request r;
	r.set_write_request(m_buffer,
						&m_byteStreamAccessor,
						m_tempFile,
						writeOffset,
						blockItems,
						blockNumber,
						&m_response);
	compressor().request(r);
	m_bufferDirty = false;
	
	if (m_updateReadOffsetFromWrite) {
		m_lastWriteBlockNumber = blockNumber;
	}
	
	if (blockNumber == m_streamBlocks) {
		++m_streamBlocks;
	}
}

void compressed_stream_base::maybe_update_read_offset(compressor_thread_lock & lock) {
	if (m_updateReadOffsetFromWrite && use_compression()) {
		while (!m_response.done()) {
			m_response.wait(lock);
		}
		if (m_response.has_block_info(m_lastWriteBlockNumber)) {
			m_readOffset = m_response.get_read_offset(m_lastWriteBlockNumber);
			m_nextReadOffset = m_readOffset + m_response.get_block_size(m_lastWriteBlockNumber);
		}
		m_updateReadOffsetFromWrite = false;
	}
}


void compressed_stream_base::read_next_block(compressor_thread_lock & lock, stream_size_type blockNumber) {
	uncache_read_writes();
	get_buffer(lock, blockNumber);
	
	maybe_update_read_offset(lock);
	
	stream_size_type readOffset;
	if (m_buffer->get_state() == compressor_buffer_state::clean) {
		m_readOffset = m_buffer->get_read_offset();
		if (use_compression()) {
			tp_assert(m_readOffset == m_nextReadOffset,
					  "read_next_block: Buffer has wrong read offset");
			m_nextReadOffset = m_readOffset + m_buffer->get_block_size();
		}
	} else {
		if (use_compression()) {
			readOffset = m_nextReadOffset;
		} else {
			stream_size_type itemOffset = blockNumber * m_blockItems;
			readOffset = blockNumber * m_blockSize;
			memory_size_type blockSize =
				std::min(m_blockSize,
						 static_cast<memory_size_type>((size() - itemOffset) * m_itemSize));
			m_buffer->set_size(blockSize);
		}
		
		read_block(lock, readOffset, read_direction::forward);
		size_t blockItems = m_blockItems;
		if (size() - blockNumber * m_blockItems < blockItems) {
			blockItems = static_cast<size_t>(size() - blockNumber * m_blockItems);
		}
		size_t usableBlockSize = m_buffer->size() / m_itemSize * m_itemSize;
		size_t expectedBlockSize = blockItems * m_itemSize;
		if (usableBlockSize != expectedBlockSize) {
			log_error() << "Expected " << expectedBlockSize << " (" << blockItems
						<< " items), got " << m_buffer->size() << " (rounded to "
						<< usableBlockSize << ')' << std::endl;
			throw exception("read_next_block: Bad buffer->get_size");
		}
		
		// Update m_readOffset, m_nextReadOffset
		if (use_compression()) {
			m_readOffset = readOffset;
			m_nextReadOffset = m_response.next_read_offset();
			if (m_readOffset != m_buffer->get_read_offset())
				throw exception("read_next_block: bad get_read_offset");
			if (m_nextReadOffset != m_readOffset + m_buffer->get_block_size())
				throw exception("read_next_block: bad get_block_size");
		} else {
			// Uncompressed case. The following is a no-op:
			//m_readOffset = 0;
			// nextReadOffset is not used.
		}
	}
	
	m_nextItem = m_bufferBegin;
}

void compressed_stream_base::read_previous_block(compressor_thread_lock & lock, stream_size_type blockNumber) {
	uncache_read_writes();
	tp_assert(use_compression(), "read_previous_block: !use_compression");
	get_buffer(lock, blockNumber);
	
	maybe_update_read_offset(lock);
	
	if (m_buffer->get_state() == compressor_buffer_state::clean) {
		m_readOffset = m_buffer->get_read_offset();
		m_nextReadOffset = m_readOffset + m_buffer->get_block_size();
	} else {
		read_block(lock, m_readOffset, read_direction::backward);
		
		// This is backwards since we are reading backwards.
		// Confusing, I know.
		m_nextReadOffset = m_readOffset;
		m_readOffset = m_response.next_read_offset();
		
		if (m_readOffset != m_buffer->get_read_offset())
			throw exception("Bad buffer get_read_offset");
		if (m_nextReadOffset != m_readOffset + m_buffer->get_block_size())
			throw exception("Bad buffer get_block_size");
	}
	
	m_nextItem = m_bufferEnd;
}

void compressed_stream_base::read_block(compressor_thread_lock & lock,
				stream_size_type readOffset,
				read_direction::type readDirection)
{
	compressor_request r;
	r.set_read_request(m_buffer,
					   &m_byteStreamAccessor,
					   readOffset,
					   readDirection,
					   &m_response);
	m_buffer->transition_state(compressor_buffer_state::dirty,
							   compressor_buffer_state::reading);
	compressor().request(r);
	while (!m_response.done()) {
		m_response.wait(lock);
	}
}

void compressed_stream_base::cache_read_writes() {
	if (m_buffer.get() == 0 || m_seekState != seek_state::none) {
		m_cachedWrites = 0;
		m_cachedReads = 0;
	} else if (offset() == size()) {
		m_cachedWrites = m_bufferDirty ? ((m_bufferEnd - m_nextItem) / m_itemSize) : 0;
		m_cachedReads = 0;
	} else {
		m_cachedWrites = 0;
		m_cachedReads = (m_bufferEnd - m_nextItem) / m_itemSize;
		if (offset() + m_cachedReads > size()) {
			m_cachedReads =
				static_cast<memory_size_type>(size() - offset());
		}
	}
}

void compressed_stream_base::peak_unlikely() {
	if (m_seekState != seek_state::none) perform_seek();
	if (m_offset == m_size) throw end_of_stream_exception();
	if (m_nextItem == m_bufferEnd) {
		compressor_thread_lock l(compressor());
		if (this->m_bufferDirty) {
			m_updateReadOffsetFromWrite = false;
			flush_block(l);
		}
		// At this point, block_number() == buffer_block_number() + 1
		read_next_block(l, block_number());
	}
}

void compressed_stream_base::read_back_unlikely() {
	if (m_seekState != seek_state::none) {
		if (offset() == 0) throw end_of_stream_exception();
		perform_seek(read_direction::backward);
	}
	if (m_nextItem == m_bufferBegin) {
		if (m_offset == 0) throw end_of_stream_exception();
		uncache_read_writes();
		compressor_thread_lock l(compressor());
		if (this->m_bufferDirty) {
			m_updateReadOffsetFromWrite = true;
			flush_block(l);
		}
		if (use_compression()) {
			read_previous_block(l, block_number() - 1);
		} else {
			read_next_block(l, block_number() - 1);
			m_nextItem = m_bufferEnd;
		}
	}
}

	
void compressed_stream_base::write_unlikely(const char * item) {
	if (m_seekState != seek_state::none) perform_seek();
	
	if (!use_compression()) {
		if (m_nextItem == m_bufferEnd) {
			compressor_thread_lock lock(compressor());
			if (m_bufferDirty) {
				m_updateReadOffsetFromWrite = true;
				flush_block(lock);
			}
			if (offset() == size()) {
				get_buffer(lock, m_streamBlocks);
				m_nextItem = m_bufferBegin;
			} else {
				read_next_block(lock, block_number());
			}
		}
		if (offset() == m_size) ++m_size;
		memcpy(m_nextItem, item, m_itemSize);
		m_nextItem += m_itemSize;
		this->m_bufferDirty = true;
		++m_offset;
		cache_read_writes();
		return;
	}
	
	if (m_offset != size())
		throw stream_exception("Non-appending write attempted");
	
	if (m_nextItem == m_bufferEnd) {
		compressor_thread_lock l(compressor());
		if (m_bufferDirty) {
			m_updateReadOffsetFromWrite = true;
			flush_block(l);
		}
		get_buffer(l, m_streamBlocks);
		m_nextItem = m_bufferBegin;
	}
	
	memcpy(m_nextItem, item, m_itemSize);
	m_nextItem += m_itemSize;
	this->m_bufferDirty = true;
	++m_size;
	++m_offset;
	
	cache_read_writes();
}

void compressed_stream_base::describe(std::ostream & out) {
	if (!this->is_open()) {
		out << "[Closed stream]";
		return;
	}
	
	out << "[(" << m_byteStreamAccessor.path() << ") item " << offset()
		<< " of " << size();
	out << " (block " << block_number()
		<< " @ byte " << m_readOffset
		<< ", item " << block_item_index()
		<< ")";
	
	if (use_compression()) {
		out << ", compressed";
	} else {
		out << ", uncompressed";
	}
	
	switch (m_seekState) {
	case seek_state::none:
		break;
	case seek_state::beginning:
		out << ", seeking to beginning";
		break;
	case seek_state::end:
		out << ", seeking to end";
		break;
	case seek_state::position:
		out << ", seeking to position " << m_nextPosition.offset();
		out << " (block " << block_number(m_nextPosition.offset())
			<< " @ byte " << m_nextPosition.read_offset()
			<< ", item " << block_item_index(m_nextPosition.offset())
			<< ")";
		break;
	}
	
	if (m_bufferDirty)
		out << " dirty";
	
	if (m_seekState == seek_state::none) {
		if (can_read()) out << ", can read";
		else out << ", cannot read";
	}
	
	out << ", " << m_streamBlocks << " blocks";
	if (m_lastBlockReadOffset != std::numeric_limits<stream_size_type>::max())
		out << ", last block at " << m_lastBlockReadOffset;
	if (m_currentFileSize != std::numeric_limits<stream_size_type>::max())
		out << ", current file size " << m_currentFileSize;
	
	out << ']';
}

void compressed_stream_base::seek(stream_offset_type offset, offset_type whence) {
	tp_assert(is_open(), "seek: !is_open");
	uncache_read_writes();
	m_updateReadOffsetFromWrite = false;
	if (!use_compression()) {
		// Handle uncompressed case by delegating to set_position.
		switch (whence) {
		case beginning:
			break;
		case end:
			offset += size();
			break;
		case current:
			offset += this->offset();
			break;
		}
		set_position(stream_position(0, offset));
		return;
	}
	// Otherwise, we are in a compressed stream.
	if (offset != 0) throw stream_exception("Random seeks are not supported");
	switch (whence) {
	case beginning:
		if (m_buffer.get() != 0 && buffer_block_number() == 0) {
			// We are already reading or writing the first block.
			m_nextItem = m_bufferBegin;
			m_offset = m_readOffset = 0;
			m_seekState = seek_state::none;
		} else {
			// We need to load the first block on the next I/O.
			m_seekState = seek_state::beginning;
		}
		return;
	case end:
		if (m_buffer.get() == 0) {
			m_seekState = seek_state::end;
		} else if (m_offset == size()) {
			// no-op
			m_seekState = seek_state::none;
		} else if (// We are in the last block, and it has NOT YET been written to disk, or
			buffer_block_number() == m_streamBlocks ||
			// we are in the last block, and it has ALREADY been written to disk.
			buffer_block_number()+1 == m_streamBlocks)
		{
			// If the last block is full,
			// block_item_index() reports 0 when it should report m_blockItems.
			// Compute blockItemIndex manually to handle this edge case.
			stream_size_type blockItemIndex =
				size() - buffer_block_number() * m_blockItems;
			memory_size_type cast = static_cast<memory_size_type>(blockItemIndex);
			tp_assert(blockItemIndex == cast, "seek: blockItemIndex out of bounds");
			m_nextItem = m_bufferBegin + cast * m_itemSize;
			
			m_offset = size();
			m_seekState = seek_state::none;
		} else {
			m_seekState = seek_state::end;
		}
		return;
	case current:
		return;
	}
	tp_assert(false, "seek: Unknown whence");
}

void compressed_stream_base::truncate(stream_size_type offset) {
	tp_assert(is_open(), "truncate: !is_open");
	uncache_read_writes();
	if (offset == size())
		return;
	else if (offset == 0)
		truncate_zero();
	else if (!use_compression())
		truncate_uncompressed(offset);
	else
		throw stream_exception("Arbitrary truncate is not supported");
	
	if (m_tempFile) m_tempFile->update_recorded_size(m_size);
}

///////////////////////////////////////////////////////////////////////////
/// \brief  Truncate to given stream position.
///////////////////////////////////////////////////////////////////////////
void compressed_stream_base::truncate(const stream_position & pos) {
	tp_assert(is_open(), "truncate: !is_open");
	uncache_read_writes();
	if (pos.offset() == size())
		return;
	else if (pos.offset() == 0)
		truncate_zero();
	else if (!use_compression())
		truncate_uncompressed(pos.offset());
	else
		truncate_compressed(pos);
	
	if (m_tempFile) m_tempFile->update_recorded_size(m_size);
}

///////////////////////////////////////////////////////////////////////////
/// \brief  Store the current stream position such that it may be found
/// later on.
///
/// The stream_position object is violated if the stream is eventually
/// truncated to before the current position.
///
/// The stream_position objects are plain old data, so they may themselves
/// be written to streams.
///
/// Blocks to take the compressor lock.
///////////////////////////////////////////////////////////////////////////
stream_position compressed_stream_base::get_position() {
	tp_assert(is_open(), "get_position: !is_open");
	if (!use_compression()) return stream_position(0, offset());
	switch (m_seekState) {
	case seek_state::position:
		// We just set_position, so we can just return what we got.
		return m_nextPosition;
	case seek_state::beginning:
		return stream_position(0, 0);
	case seek_state::none:
		if (buffer_block_number() != m_streamBlocks) {
			if (m_nextItem == m_bufferEnd)
				return stream_position(m_nextReadOffset, m_offset);
			else
				return stream_position(m_readOffset, m_offset);
		}
		// We are in a new block at the end of the stream.
		if (m_nextItem == m_bufferEnd) {
			tp_assert(m_bufferDirty, "At end of buffer, but bufferDirty is false?");
			// Make sure the position we get is not at the end of a block
			compressor_thread_lock lock(compressor());
			m_updateReadOffsetFromWrite = false;
			flush_block(lock);
			get_buffer(lock, m_streamBlocks);
			m_nextItem = m_bufferBegin;
		}
		break;
	case seek_state::end:
		// Figure out the size of the file below.
		break;
	}
	
	stream_size_type readOffset;
	stream_size_type blockNumber = block_number(offset());
	compressor_thread_lock l(compressor());
	if (size() % m_blockItems == 0)
		readOffset = current_file_size(l);
	else if (blockNumber == m_streamBlocks)
		readOffset = current_file_size(l);
	else if (blockNumber == m_streamBlocks - 1)
		readOffset = last_block_read_offset(l);
	else {
		tp_assert(false, "get_position: Invalid block_number");
		readOffset = 1111111111111111111ull; // avoid compiler warning
	}
	return stream_position(readOffset, offset());
}

///////////////////////////////////////////////////////////////////////////
/// \brief  Seek to a position that was previously recalled with
/// \c get_position.
///////////////////////////////////////////////////////////////////////////
void compressed_stream_base::set_position(const stream_position & pos) {
	m_updateReadOffsetFromWrite = false;
	
	// If the code is correct, short circuiting is not necessary;
	// if the code is not correct, short circuiting might mask faults.
	/*
	  if (pos == m_position) {
	  m_seekState = seek_state::none;
	  return;
	  }
	*/
	
	if (pos == stream_position::end()) {
		seek(0, end);
		return;
	}
	
	if (!use_compression() && pos.read_offset() != 0)
		throw stream_exception("set_position: Invalid position, read_offset != 0");
	
	if (pos.offset() > size())
		throw stream_exception("set_position: Invalid position, offset > size");
	
	if (m_buffer.get() != 0
		&& block_number(pos.offset()) == buffer_block_number())
	{
		if (pos.read_offset() != m_readOffset) {
			// We don't always know the read offset of the current block
			// in m_readOffset, so let's assume that
			// pos.read_offset() is correct.
		}
		
		m_readOffset = pos.read_offset();
		m_offset = pos.offset();
		m_nextItem = m_bufferBegin + m_itemSize * block_item_index();
		m_seekState = seek_state::none;
		return;
	}
	
	m_nextPosition = pos;
	m_seekState = seek_state::position;
	uncache_read_writes();
}


///////////////////////////////////////////////////////////////////////////////
/// \brief  Implementation helper that closes the stream if a method exits
/// by throwing an exception.
///
/// At every proper exit point from the method, commit() must be called.
/// If the method exits without committing, close() is called.
/// Care should be taken to ensure that the compressor lock is not held when
/// this object is destructed!
///////////////////////////////////////////////////////////////////////////////
class close_on_fail_guard {
public:
	close_on_fail_guard(compressed_stream_base * s)
		: m_committed(false)
		, m_stream(s)
	{
	}

	~close_on_fail_guard() {
		if (!m_committed) m_stream->close();
	}

	void commit() {
		m_committed = true;
	}

private:
	bool m_committed;
	compressed_stream_base * m_stream;
};

void compressed_stream_base::perform_seek(read_direction::type dir) {
	if (!is_open()) throw stream_exception("Stream is not open");
	// This must be initialized before the compressor lock below,
	// so that it is destructed after we free the lock.
	close_on_fail_guard closeOnFail(this);
	
	tp_assert(!(m_seekState == seek_state::none), "perform_seek when seekState is none");
	
	uncache_read_writes();
	
	compressor_thread_lock l(compressor());
	
	m_updateReadOffsetFromWrite = false;
	
	if (this->m_bufferDirty)
		flush_block(l);
	
	m_buffer.reset();
	finish_requests(l);

	// Ensure that seek state beginning will take us to a read-only state
	if (m_seekState == seek_state::beginning && size() == 0) {
		m_seekState = seek_state::end;
	}
	
	// Ensure that seek state position will take us to a read-only state
	if (m_seekState == seek_state::position
		&& m_nextPosition.offset() == size())
	{
		m_seekState = seek_state::end;
	}
	
	if (m_seekState == seek_state::beginning) {
		// The (seek beginning && size() == 0) case is handled
		// by changing seekState to end.
		// Thus, we know for sure that size() != 0, and so the
		// read_next_block will not yield an end_of_stream_exception.
		tp_assert(!(size() == 0), "Seek beginning when size is zero");
		if (use_compression()) {
			m_nextReadOffset = 0;
		}
		read_next_block(l, 0);
		m_offset = 0;
		tp_assert(m_readOffset == 0, "perform_seek: Bad readOffset after reading first block");
	} else if (m_seekState == seek_state::position) {
		stream_size_type blockNumber = block_number(m_nextPosition.offset());
		memory_size_type blockItemIndex = block_item_index(m_nextPosition.offset());
		
		// This cannot happen in practice due to the implementation of
		// block_number and block_item_index, but it is an important
		// assumption in the following code.
		tp_assert(!(blockItemIndex >= m_blockItems), "perform_seek: Computed block item index >= blockItems");
		
		if (dir == read_direction::backward && blockItemIndex == 0 && blockNumber > 0) {
			if (use_compression()) {
				m_readOffset = m_nextPosition.read_offset();
				read_previous_block(l, blockNumber - 1);
				// sets m_nextItem = m_bufferEnd
			} else {
				read_next_block(l, blockNumber - 1);
				m_nextItem = m_bufferEnd;
			}
		} else {
			if (use_compression()) {
				m_nextReadOffset = m_nextPosition.read_offset();
			}
			read_next_block(l, blockNumber);
			m_nextItem = m_bufferBegin + m_itemSize * blockItemIndex;
		}
		
		m_offset = m_nextPosition.offset();
	} else if (m_seekState == seek_state::end) {
		if (m_streamBlocks * m_blockItems == size() && dir == read_direction::forward) {
			// The last block in the stream is full,
			// so we can safely start a new empty one.
			get_buffer(l, m_streamBlocks);
			m_nextItem = m_bufferBegin;
			if (use_compression()) {
				m_readOffset = current_file_size(l);
			} else {
				m_readOffset = 0;
			}
			m_offset = size();
		} else {
			// The last block in the stream is non-full, or we are going to read_back.
			if (m_streamBlocks == 0) {
				// This cannot happen in practice,
				// since we short-circuit seek(end) when streamBlocks == 0.
				throw exception("Attempted seek to end when no blocks have been written");
			}
			memory_size_type blockItemIndex =
				static_cast<memory_size_type>(size() - (m_streamBlocks - 1) * m_blockItems);
			if (use_compression()) {
				m_nextReadOffset = last_block_read_offset(l);
			}
			read_next_block(l, m_streamBlocks - 1);
			m_nextItem = m_bufferBegin + m_itemSize * blockItemIndex;
			m_offset = size();
		}
	} else {
		log_debug() << "Unknown seek state " << m_seekState << std::endl;
		tp_assert(false, "perform_seek: Unknown seek state");
	}
	
	m_seekState = seek_state::none;

	closeOnFail.commit();
}



} // namespace tpie
