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
#include <tpie/file_accessor/byte_stream_accessor.h>
#include <tpie/compressed/thread.h>
#include <tpie/compressed/buffer.h>
#include <tpie/compressed/request.h>
#include <tpie/compressed/direction.h>

namespace tpie {


open::type translate(access_type accessType, cache_hint cacheHint, compression_flags compressionFlags) {
	return (open::type) ((
							 (accessType == access_read) ? open::read_only :
							 (accessType == access_write) ? open::write_only :
							 open::defaults) | (
								 
								 (cacheHint == tpie::access_normal) ? open::access_normal :
								 (cacheHint == tpie::access_random) ? open::access_random :
								 open::defaults) | (
									 
									 (compressionFlags == tpie::compression_normal) ? open::compression_normal :
									 (compressionFlags == tpie::compression_all) ? open::compression_all :
									 open::defaults));
}

cache_hint translate_cache(open::type openFlags) {
	const open::type cacheFlags =
		openFlags & (open::access_normal | open::access_random);
	
	if (cacheFlags == open::access_normal)
		return tpie::access_normal;
	else if (cacheFlags == open::access_random)
		return tpie::access_random;
	else if (!cacheFlags)
		return tpie::access_sequential;
	else
		throw tpie::stream_exception("Invalid cache flags supplied");
}

compression_flags translate_compression(open::type openFlags) {
	const open::type compressionFlags =
		openFlags & (open::compression_normal | open::compression_all);
	
	if (compressionFlags == open::compression_normal)
		return tpie::compression_normal;
	else if (compressionFlags == open::compression_all)
		return tpie::compression_all;
	else if (!compressionFlags)
		return tpie::compression_none;
	else
		throw tpie::stream_exception("Invalid compression flags supplied");
}

typedef std::shared_ptr<compressor_buffer> buffer_t;

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


class compressed_stream_base_p {
public:
	/** Whether the current block must be written out to disk before being ejected.
	 * Invariants:
	 * If m_bufferDirty is true and use_compression() is true,
	 * block_number() is either m_streamBlocks or m_streamBlocks - 1.
	 * If block_number() is m_streamBlocks, m_bufferDirty is true.
	 */
	bool m_bufferDirty;
	/** Number of items in a logical block. */
	memory_size_type m_blockItems;
	/** Size (in bytes) of a logical (uncompressed) block. */
	memory_size_type m_blockSize;
	/** Whether we are open for reading. */
	bool m_canRead;
	/** Whether we are open for writing. */
	bool m_canWrite;
	/** Whether we are open. */
	bool m_open;
	/** Size of a single item. itemSize * blockItems == blockSize. */
	memory_size_type m_itemSize;

	/** The anonymous temporary file we have opened (when appropriate). */
	tpie::unique_ptr<temp_file> m_ownedTempFile;
	/** The temporary file we have opened (when appropriate).
	 * When m_ownedTempFile.get() != 0, m_tempFile == m_ownedTempFile.get(). */
	temp_file * m_tempFile;
	/** File accessor. */
	file_accessor::byte_stream_accessor<default_raw_file_accessor> m_byteStreamAccessor;

	stream_buffers m_buffers;
	/** Buffer holding the items of the block currently being read/written. */
	buffer_t m_buffer;

	/** The number of blocks written to the file.
	 * We must always have (m_streamBlocks+1) * m_blockItems <= m_size. */
	stream_size_type m_streamBlocks;

	/** When use_compression() is true:
	 * Read offset of the last block in the stream.
	 * Necessary to support seeking to the end. */
	stream_size_type m_lastBlockReadOffset;
	stream_size_type m_currentFileSize;

	/** Response from compressor thread; protected by compressor thread mutex. */
	compressor_response m_response;

	/** When use_compression() is true:
	 * Indicates whether m_response is the response to a write request.
	 * Used for knowing where to read next in read/read_back.
	 * */
	bool m_updateReadOffsetFromWrite = false;
	stream_size_type m_lastWriteBlockNumber;

	/** Position relating to the currently loaded buffer.
	 * readOffset is only valid during reading.
	 * Invariants:
	 *
	 * If use_compression() == false, readOffset == 0.
	 * If offset == 0, then readOffset == block_item_index() == block_number() == 0.
	 */
	stream_size_type m_readOffset;

	
	/** If seekState is `position`, seek to this position before reading/writing. */
	stream_position m_nextPosition;

	stream_size_type m_nextReadOffset;

	compressed_stream_base * m_o;
	
	compressed_stream_base_p(memory_size_type itemSize, double blockFactor, compressed_stream_base * outer)
		: m_bufferDirty(false)
		, m_blockItems(block_size(blockFactor) / itemSize)
		, m_blockSize(block_size(blockFactor))
		, m_canRead(false)
		, m_canWrite(false)
		, m_open(false)
		, m_itemSize(itemSize)
		, m_ownedTempFile(/* empty unique_ptr */)
		, m_tempFile(0)
		, m_byteStreamAccessor()
		, m_buffers(m_blockSize)
		, m_buffer(/* empty shared_ptr */)
		, m_streamBlocks(0)
		, m_lastBlockReadOffset(0)
		, m_currentFileSize(0)
		, m_response()
		, m_readOffset(0)
		, m_nextPosition(/* not a position */)
		, m_nextReadOffset(0)
		, m_o(outer)
		{}

	void open_inner(const std::string & path,
					open::type openFlags,
					memory_size_type userDataSize) {
		// Parse openFlags
		const bool readOnly = openFlags & open::read_only;
		const bool writeOnly = openFlags & open::write_only;
		if (readOnly && writeOnly)
			throw tpie::stream_exception("Invalid read/write only flags");
		m_canRead = !writeOnly;
		m_canWrite = !readOnly;
		
		const cache_hint cacheHint = translate_cache(openFlags);
		const compression_flags compressionFlags = translate_compression(openFlags);
		
		m_byteStreamAccessor.open(path, m_canRead, m_canWrite, m_itemSize,
								  m_blockSize, userDataSize, cacheHint,
								  compressionFlags);
		m_o->m_size = m_byteStreamAccessor.size();
		m_open = true;
		m_streamBlocks = (m_o->m_size + m_blockItems - 1) / m_blockItems;
		m_lastBlockReadOffset = m_byteStreamAccessor.get_last_block_read_offset();
		m_currentFileSize = m_byteStreamAccessor.file_size();
		m_response.clear_block_info();
		
		m_o->seek(0);
	}

	static memory_size_type block_size(double blockFactor) noexcept {
		return static_cast<memory_size_type>(get_block_size() * blockFactor);
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Reset cheap read/write counts to zero so that the next
	/// read/write operation will check stream state properly.
	///////////////////////////////////////////////////////////////////////////
	void uncache_read_writes() {
		m_o->m_cachedReads = m_o->m_cachedWrites = 0;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	///
	/// Precondition: m_bufferDirty == true.
	/// Postcondition: m_bufferDirty == false.
	///
	/// Does not get a new block buffer.
	///////////////////////////////////////////////////////////////////////////
	void flush_block(compressor_thread_lock & lock) {
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
		
		if (m_o->m_nextItem == nullptr) throw exception("m_nextItem is NULL");
		if (m_o->m_bufferBegin == nullptr) throw exception("m_bufferBegin is NULL");
		memory_size_type blockItems = m_blockItems;
		if (blockItems + blockNumber * m_blockItems > m_o->m_size) {
			blockItems =
				static_cast<memory_size_type>(m_o->m_size - blockNumber * m_blockItems);
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


	stream_size_type block_number(stream_size_type offset) {
		return offset / m_blockItems;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute the number of the block containing the next
	/// read/written item.
	///
	/// Precondition: m_buffer.get() != 0.
	/// Precondition: m_seekState == none
	///////////////////////////////////////////////////////////////////////////
	stream_size_type block_number() {
		return block_number(m_o->m_offset);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute the number of the block currently loaded into m_buffer.
	///
	/// Precondition: m_buffer.get() != 0.
	/// Precondition: m_seekState == none
	///////////////////////////////////////////////////////////////////////////
	stream_size_type buffer_block_number() {
		stream_size_type blockNumber = block_number();
		if (m_o->m_nextItem == m_o->m_bufferEnd)
			return blockNumber - 1;
		else
			return blockNumber;
	}

	memory_size_type block_item_index(stream_size_type offset) {
		stream_size_type i = offset % m_blockItems;
		memory_size_type cast = static_cast<memory_size_type>(i);
		tp_assert(!(i != cast), "Block item index out of bounds");
		return cast;
	}

	memory_size_type block_item_index() {
		return block_item_index(m_o->m_offset);
	}
	
	bool use_compression() { return m_byteStreamAccessor.get_compressed(); }

	///////////////////////////////////////////////////////////////////////////
 	/// Blocks to take the compressor lock.
 	///
 	/// Precondition: use_compression()
 	///
 	/// TODO: Should probably investigate when this reports a useful value.
 	///////////////////////////////////////////////////////////////////////////
	stream_size_type last_block_read_offset(compressor_thread_lock & l) {
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

	void finish_requests(compressor_thread_lock & l) {
		tp_assert(!(m_buffer.get() != 0), "finish_requests called when own buffer is still held");
		m_buffers.clean();
		while (!m_buffers.empty()) {
			compressor().wait_for_request_done(l);
			m_buffers.clean();
		}
	}

	compressor_thread & compressor() { return the_compressor_thread(); }

	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	///
	/// Precondition: use_compression()
	///
	/// TODO: Should probably investigate when this reports a useful value.
	///////////////////////////////////////////////////////////////////////////
	stream_size_type current_file_size(compressor_thread_lock & l) {
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


	///////////////////////////////////////////////////////////////////////////
	/// \brief  Truncate to zero size.
	///////////////////////////////////////////////////////////////////////////
	void truncate_zero() {
		// No need to flush block
		m_buffer.reset();
		m_response.clear_block_info();
		m_updateReadOffsetFromWrite = false;
		compressor_thread_lock l(compressor());
		finish_requests(l);
		get_buffer(l, 0);
		m_o->m_size = 0;
		m_streamBlocks = 0;
		m_byteStreamAccessor.truncate(0);
	
		m_readOffset = 0;
		m_o->m_offset = 0;
		m_o->m_nextItem = m_o->m_bufferBegin;
		m_o->m_seekState = compressed_stream_base::seek_state::none;
		uncache_read_writes();
	}

	void truncate_uncompressed(stream_size_type offset) {
		tp_assert(!use_compression(), "truncate_uncompressed called on compressed stream");
	
		stream_size_type currentOffset = m_o->offset();
		if (m_buffer.get() != 0
			&& block_number(offset) == buffer_block_number()
			&& buffer_block_number() == m_streamBlocks)
		{
			// We are truncating a final block that has not been written yet.
			m_o->m_size = offset;
			if (offset < m_o->m_offset) {
				m_o->m_offset = offset;
				memory_size_type blockItemIndex =
					static_cast<memory_size_type>(offset - m_streamBlocks * m_blockItems);
				m_o->m_nextItem = m_o->m_bufferBegin + m_itemSize * blockItemIndex;
			}
			m_bufferDirty = true;
			m_o->m_seekState = compressed_stream_base::seek_state::none;
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
			m_o->m_size = offset;
			m_streamBlocks = (offset + m_blockItems - 1) / m_blockItems;
		}
		m_o->seek(std::min(currentOffset, offset));
	}

	void truncate_compressed(const stream_position & pos) {
		tp_assert(use_compression(), "truncate_compressed called on uncompressed stream");
	
		stream_size_type offset = pos.offset();
		stream_position finalDestination = (offset < m_o->offset()) ? pos : m_o->get_position();
	
		if (m_buffer.get() == 0 || block_number(offset) != buffer_block_number()) {
			m_o->set_position(pos);
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
		m_o->m_size = offset;
		if (offset < m_o->m_offset) {
			m_o->m_offset = offset;
			memory_size_type blockItemIndex =
				static_cast<memory_size_type>(offset - m_streamBlocks * m_blockItems);
			m_o->m_nextItem = m_o->m_bufferBegin + m_itemSize * blockItemIndex;
		}
		m_bufferDirty = true;
	
		m_o->set_position(finalDestination);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Gets buffer for given block and sets bufferBegin and bufferEnd,
	/// and sets bufferDirty to false.
	///////////////////////////////////////////////////////////////////////////
	void get_buffer(compressor_thread_lock & l, stream_size_type blockNumber) {
		uncache_read_writes();
		buffer_t().swap(m_buffer);
		m_buffer = this->m_buffers.get_buffer(l, blockNumber);
		while (m_buffer->is_busy()) compressor().wait_for_request_done(l);
		m_o->m_bufferBegin = m_buffer->get();
		m_o->m_bufferEnd = m_o->m_bufferBegin + m_itemSize * m_blockItems;
		this->m_bufferDirty = false;
	}


	void maybe_update_read_offset(compressor_thread_lock & lock) {
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

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Reads next block according to nextReadOffset/nextBlockSize.
	///
	/// Updates m_readOffset with the new read offset.
	///////////////////////////////////////////////////////////////////////////
	void read_next_block(compressor_thread_lock & lock, stream_size_type blockNumber) {
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
							 static_cast<memory_size_type>((m_o->size() - itemOffset) * m_itemSize));
				m_buffer->set_size(blockSize);
			}
		
			read_block(lock, readOffset, read_direction::forward);
			size_t blockItems = m_blockItems;
			if (m_o->size() - blockNumber * m_blockItems < blockItems) {
				blockItems = static_cast<size_t>(m_o->size() - blockNumber * m_blockItems);
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
	
		m_o->m_nextItem = m_o->m_bufferBegin;
	}

	void read_previous_block(compressor_thread_lock & lock, stream_size_type blockNumber) {
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
	
		m_o->m_nextItem = m_o->m_bufferEnd;
	}

	void read_block(compressor_thread_lock & lock,
											stream_size_type readOffset,
											read_direction::type readDirection) {
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

	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	///
	/// Precondition: seekState != none
	///
	/// Sets seekState to none.
	///
	/// If anything fails, the stream is closed by a close_on_fail_guard.
	///////////////////////////////////////////////////////////////////////////
	void perform_seek(read_direction::type dir=read_direction::forward) {
		if (!m_open) throw stream_exception("Stream is not open");
		// This must be initialized before the compressor lock below,
		// so that it is destructed after we free the lock.
		close_on_fail_guard closeOnFail(m_o);
	
		tp_assert(!(m_o->m_seekState == compressed_stream_base::seek_state::none), "perform_seek when seekState is none");
	
		uncache_read_writes();
	
		compressor_thread_lock l(compressor());
	
		m_updateReadOffsetFromWrite = false;
	
		if (this->m_bufferDirty)
			flush_block(l);
	
		m_buffer.reset();
		finish_requests(l);

		// Ensure that seek state beginning will take us to a read-only state
		if (m_o->m_seekState == compressed_stream_base::seek_state::beginning && m_o->size() == 0) {
			m_o->m_seekState = compressed_stream_base::seek_state::end;
		}
	
		// Ensure that seek state position will take us to a read-only state
		if (m_o->m_seekState == compressed_stream_base::seek_state::position
			&& m_nextPosition.offset() == m_o->size())
		{
			m_o->m_seekState = compressed_stream_base::seek_state::end;
		}
	
		if (m_o->m_seekState == compressed_stream_base::seek_state::beginning) {
			// The (seek beginning && size() == 0) case is handled
			// by changing seekState to end.
			// Thus, we know for sure that size() != 0, and so the
			// read_next_block will not yield an end_of_stream_exception.
			tp_assert(!(m_o->size() == 0), "Seek beginning when size is zero");
			if (use_compression()) {
				m_nextReadOffset = 0;
			}
			read_next_block(l, 0);
			m_o->m_offset = 0;
			tp_assert(m_readOffset == 0, "perform_seek: Bad readOffset after reading first block");
		} else if (m_o->m_seekState == compressed_stream_base::seek_state::position) {
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
					m_o->m_nextItem = m_o->m_bufferEnd;
				}
			} else {
				if (use_compression()) {
					m_nextReadOffset = m_nextPosition.read_offset();
				}
				read_next_block(l, blockNumber);
				m_o->m_nextItem = m_o->m_bufferBegin + m_itemSize * blockItemIndex;
			}
			
			m_o->m_offset = m_nextPosition.offset();
		} else if (m_o->m_seekState == compressed_stream_base::seek_state::end) {
			if (m_streamBlocks * m_blockItems == m_o->size() && dir == read_direction::forward) {
				// The last block in the stream is full,
				// so we can safely start a new empty one.
				get_buffer(l, m_streamBlocks);
				m_o->m_nextItem = m_o->m_bufferBegin;
				if (use_compression()) {
					m_readOffset = current_file_size(l);
				} else {
					m_readOffset = 0;
				}
				m_o->m_offset = m_o->size();
			} else {
				// The last block in the stream is non-full, or we are going to read_back.
				if (m_streamBlocks == 0) {
					// This cannot happen in practice,
					// since we short-circuit seek(end) when streamBlocks == 0.
					throw exception("Attempted seek to end when no blocks have been written");
				}
				memory_size_type blockItemIndex =
					static_cast<memory_size_type>(m_o->size() - (m_streamBlocks - 1) * m_blockItems);
				if (use_compression()) {
					m_nextReadOffset = last_block_read_offset(l);
				}
				read_next_block(l, m_streamBlocks - 1);
				m_o->m_nextItem = m_o->m_bufferBegin + m_itemSize * blockItemIndex;
				m_o->m_offset = m_o->size();
			}
		} else {
			log_debug() << "Unknown seek state " << m_o->m_seekState << std::endl;
			tp_assert(false, "perform_seek: Unknown seek state");
		}
	
		m_o->m_seekState = compressed_stream_base::seek_state::none;

		closeOnFail.commit();
	}
	
};
compressed_stream_base::compressed_stream_base(memory_size_type itemSize,
											   double blockFactor)
	: m_cachedReads(0)
	, m_cachedWrites(0)
	, m_size(0)
	, m_seekState(seek_state::beginning)
	, m_offset(0)
	, m_bufferBegin(nullptr)
	, m_bufferEnd(nullptr)
	, m_nextItem(nullptr)
	, m_p(new compressed_stream_base_p(itemSize, blockFactor, this)) 
{
	// Empty constructor.
}

compressed_stream_base::~compressed_stream_base() {
	try {
		close();
	} catch (std::exception & e) {
		log_error() << "Someone threw an error in file_stream::~file_stream: " << e.what() << std::endl;
		abort();
	}
	delete m_p;
	// non-trivial field destructors:
	// m_response::~compressor_response()
	// m_buffer::~shared_ptr()
	// m_buffers::~stream_buffers()
	// m_byteStreamAccessor::~byte_stream_accessor()
	// m_ownedTempFile::~unique_ptr()
}

memory_size_type compressed_stream_base::memory_usage(double blockFactor) noexcept {
	// m_buffer is included in m_buffers memory usage
	return sizeof(temp_file) // m_ownedTempFile
		+ stream_buffers::memory_usage(block_size(blockFactor)) // m_buffers;
		+ sizeof(compressed_stream_base_p);
}

memory_size_type compressed_stream_base::block_size(double blockFactor) noexcept {
	return compressed_stream_base_p::block_size(blockFactor);
}

double compressed_stream_base::calculate_block_factor(memory_size_type blockSize) noexcept {
	return (double)blockSize / (double)block_size(1.0);
}

memory_size_type compressed_stream_base::block_memory_usage(double blockFactor) noexcept {
	return block_size(blockFactor);
}

memory_size_type compressed_stream_base::block_items() const {
	return m_p->m_blockItems;
}

memory_size_type compressed_stream_base::block_size() const {
	return m_p->m_blockSize;
}

memory_size_type compressed_stream_base::read_user_data(void * data, memory_size_type count) {
	tp_assert(is_open(), "read_user_data: !is_open");
	return m_p->m_byteStreamAccessor.read_user_data(data, count);
}

void compressed_stream_base::write_user_data(const void * data, memory_size_type count) {
	tp_assert(is_open(), "write_user_data: !is_open");
	m_p->m_byteStreamAccessor.write_user_data(data, count);
}

memory_size_type compressed_stream_base::user_data_size() const {
	tp_assert(is_open(), "user_data_size: !is_open");
	return m_p->m_byteStreamAccessor.user_data_size();
}

memory_size_type compressed_stream_base::max_user_data_size() const {
	tp_assert(is_open(), "max_user_data_size: !is_open");
	return m_p->m_byteStreamAccessor.max_user_data_size();
}

const std::string & compressed_stream_base::path() const {
	assert(m_p->m_open);
	return m_p->m_byteStreamAccessor.path();
}

void compressed_stream_base::open(const std::string & path, open::type openFlags,
								  memory_size_type userDataSize /*= 0*/)
{
	close();
	m_p->open_inner(path, openFlags, userDataSize);
}

void compressed_stream_base::open(open::type openFlags,
								  memory_size_type userDataSize /*= 0*/)
{
	close();
	m_p->m_ownedTempFile.reset(tpie_new<temp_file>());
	m_p->m_tempFile = m_p->m_ownedTempFile.get();
	m_p->open_inner(m_p->m_tempFile->path(), openFlags, userDataSize);
}

void compressed_stream_base::open(temp_file & file, open::type openFlags,
								  memory_size_type userDataSize /*= 0*/)
{
	close();
	m_p->m_tempFile = &file;
	m_p->open_inner(m_p->m_tempFile->path(), openFlags, userDataSize);
}

void compressed_stream_base::close() {
	m_p->uncache_read_writes();
	if (m_p->m_open) {
		compressor_thread_lock l(m_p->compressor());

		if (m_p->m_bufferDirty)
			m_p->flush_block(l);

		m_p->m_buffer.reset();

		m_p->finish_requests(l);

		if (m_p->use_compression()) {
			m_p->m_byteStreamAccessor.set_last_block_read_offset(m_p->last_block_read_offset(l));
		}
		m_p->m_byteStreamAccessor.set_size(m_size);
		m_p->m_byteStreamAccessor.close();
	}
	m_p->m_open = false;
	m_p->m_tempFile = NULL;
	m_p->m_ownedTempFile.reset();

	// If read/write is called on the closed stream,
	// perform_seek throws an exception.
	m_seekState = seek_state::beginning;
}




void compressed_stream_base::cache_read_writes() {
	if (m_p->m_buffer.get() == 0 || m_seekState != seek_state::none) {
		m_cachedWrites = 0;
		m_cachedReads = 0;
	} else if (offset() == size()) {
		m_cachedWrites = m_p->m_bufferDirty ? ((m_bufferEnd - m_nextItem) / m_p->m_itemSize) : 0;
		m_cachedReads = 0;
	} else {
		m_cachedWrites = 0;
		m_cachedReads = (m_bufferEnd - m_nextItem) / m_p->m_itemSize;
		if (offset() + m_cachedReads > size()) {
			m_cachedReads =
				static_cast<memory_size_type>(size() - offset());
		}
	}
}

void compressed_stream_base::peak_unlikely() {
	if (m_seekState != seek_state::none) m_p->perform_seek();
	if (m_offset == m_size) throw end_of_stream_exception();
	if (m_nextItem == m_bufferEnd) {
		compressor_thread_lock l(m_p->compressor());
		if (m_p->m_bufferDirty) {
			m_p->m_updateReadOffsetFromWrite = false;
			m_p->flush_block(l);
		}
		// At this point, block_number() == buffer_block_number() + 1
		m_p->read_next_block(l, m_p->block_number());
	}
}

void compressed_stream_base::read_back_unlikely() {
	if (m_seekState != seek_state::none) {
		if (offset() == 0) throw end_of_stream_exception();
		m_p->perform_seek(read_direction::backward);
	}
	if (m_nextItem == m_bufferBegin) {
		if (m_offset == 0) throw end_of_stream_exception();
		m_p->uncache_read_writes();
		compressor_thread_lock l(m_p->compressor());
		if (m_p->m_bufferDirty) {
			m_p->m_updateReadOffsetFromWrite = true;
			m_p->flush_block(l);
		}
		if (m_p->use_compression()) {
			m_p->read_previous_block(l, m_p->block_number() - 1);
		} else {
			m_p->read_next_block(l, m_p->block_number() - 1);
			m_nextItem = m_bufferEnd;
		}
	}
}

	
void compressed_stream_base::write_unlikely(const char * item) {
	if (m_seekState != seek_state::none) m_p->perform_seek();
	
	if (!m_p->use_compression()) {
		if (m_nextItem == m_bufferEnd) {
			compressor_thread_lock lock(m_p->compressor());
			if (m_p->m_bufferDirty) {
				m_p->m_updateReadOffsetFromWrite = true;
				m_p->flush_block(lock);
			}
			if (offset() == size()) {
				m_p->get_buffer(lock, m_p->m_streamBlocks);
				m_nextItem = m_bufferBegin;
			} else {
				m_p->read_next_block(lock, m_p->block_number());
			}
		}
		if (offset() == m_size) ++m_size;
		memcpy(m_nextItem, item, m_p->m_itemSize);
		m_nextItem += m_p->m_itemSize;
		m_p->m_bufferDirty = true;
		++m_offset;
		cache_read_writes();
		return;
	}
	
	if (m_offset != size())
		throw stream_exception("Non-appending write attempted");
	
	if (m_nextItem == m_bufferEnd) {
		compressor_thread_lock l(m_p->compressor());
		if (m_p->m_bufferDirty) {
			m_p->m_updateReadOffsetFromWrite = true;
			m_p->flush_block(l);
		}
		m_p->get_buffer(l, m_p->m_streamBlocks);
		m_nextItem = m_bufferBegin;
	}
	
	memcpy(m_nextItem, item, m_p->m_itemSize);
	m_nextItem += m_p->m_itemSize;
	m_p->m_bufferDirty = true;
	++m_size;
	++m_offset;
	
	cache_read_writes();
}

void compressed_stream_base::describe(std::ostream & out) {
	if (!this->is_open()) {
		out << "[Closed stream]";
		return;
	}
	
	out << "[(" << m_p->m_byteStreamAccessor.path() << ") item " << offset()
		<< " of " << size();
	out << " (block " << m_p->block_number()
		<< " @ byte " << m_p->m_readOffset
		<< ", item " << m_p->block_item_index()
		<< ")";
	
	if (m_p->use_compression()) {
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
		out << ", seeking to position " << m_p->m_nextPosition.offset();
		out << " (block " << m_p->block_number(m_p->m_nextPosition.offset())
			<< " @ byte " << m_p->m_nextPosition.read_offset()
			<< ", item " << m_p->block_item_index(m_p->m_nextPosition.offset())
			<< ")";
		break;
	}
	
	if (m_p->m_bufferDirty)
		out << " dirty";
	
	if (m_seekState == seek_state::none) {
		if (can_read()) out << ", can read";
		else out << ", cannot read";
	}
	
	out << ", " << m_p->m_streamBlocks << " blocks";
	if (m_p->m_lastBlockReadOffset != std::numeric_limits<stream_size_type>::max())
		out << ", last block at " << m_p->m_lastBlockReadOffset;
	if (m_p->m_currentFileSize != std::numeric_limits<stream_size_type>::max())
		out << ", current file size " << m_p->m_currentFileSize;
	
	out << ']';
}

void compressed_stream_base::seek(stream_offset_type offset, offset_type whence) {
	tp_assert(is_open(), "seek: !is_open");
	m_p->uncache_read_writes();
	m_p->m_updateReadOffsetFromWrite = false;
	if (!m_p->use_compression()) {
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
		if (m_p->m_buffer.get() != 0 && m_p->buffer_block_number() == 0) {
			// We are already reading or writing the first block.
			m_nextItem = m_bufferBegin;
			m_offset = m_p->m_readOffset = 0;
			m_seekState = seek_state::none;
		} else {
			// We need to load the first block on the next I/O.
			m_seekState = seek_state::beginning;
		}
		return;
	case end:
		if (m_p->m_buffer.get() == 0) {
			m_seekState = seek_state::end;
		} else if (m_offset == size()) {
			// no-op
			m_seekState = seek_state::none;
		} else if (// We are in the last block, and it has NOT YET been written to disk, or
			m_p->buffer_block_number() == m_p->m_streamBlocks ||
			// we are in the last block, and it has ALREADY been written to disk.
			m_p->buffer_block_number()+1 == m_p->m_streamBlocks)
		{
			// If the last block is full,
			// block_item_index() reports 0 when it should report m_blockItems.
			// Compute blockItemIndex manually to handle this edge case.
			stream_size_type blockItemIndex =
				size() - m_p->buffer_block_number() * m_p->m_blockItems;
			memory_size_type cast = static_cast<memory_size_type>(blockItemIndex);
			tp_assert(blockItemIndex == cast, "seek: blockItemIndex out of bounds");
			m_nextItem = m_bufferBegin + cast * m_p->m_itemSize;
			
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
	m_p->uncache_read_writes();
	if (offset == size())
		return;
	else if (offset == 0)
		m_p->truncate_zero();
	else if (!m_p->use_compression())
		m_p->truncate_uncompressed(offset);
	else
		throw stream_exception("Arbitrary truncate is not supported");
	
	if (m_p->m_tempFile) m_p->m_tempFile->update_recorded_size(m_size);
}

void compressed_stream_base::truncate(const stream_position & pos) {
	tp_assert(is_open(), "truncate: !is_open");
	m_p->uncache_read_writes();
	if (pos.offset() == size())
		return;
	else if (pos.offset() == 0)
		m_p->truncate_zero();
	else if (!m_p->use_compression())
		m_p->truncate_uncompressed(pos.offset());
	else
		m_p->truncate_compressed(pos);
	
	if (m_p->m_tempFile) m_p->m_tempFile->update_recorded_size(m_size);
}

stream_position compressed_stream_base::get_position() {
	tp_assert(is_open(), "get_position: !is_open");
	if (!m_p->use_compression()) return stream_position(0, offset());
	switch (m_seekState) {
	case seek_state::position:
		// We just set_position, so we can just return what we got.
		return m_p->m_nextPosition;
	case seek_state::beginning:
		return stream_position(0, 0);
	case seek_state::none:
		if (m_p->buffer_block_number() != m_p->m_streamBlocks) {
			if (m_nextItem == m_bufferEnd)
				return stream_position(m_p->m_nextReadOffset, m_offset);
			else
				return stream_position(m_p->m_readOffset, m_offset);
		}
		// We are in a new block at the end of the stream.
		if (m_nextItem == m_bufferEnd) {
			tp_assert(m_p->m_bufferDirty, "At end of buffer, but bufferDirty is false?");
			// Make sure the position we get is not at the end of a block
			compressor_thread_lock lock(m_p->compressor());
			m_p->m_updateReadOffsetFromWrite = false;
			m_p->flush_block(lock);
			m_p->get_buffer(lock, m_p->m_streamBlocks);
			m_nextItem = m_bufferBegin;
		}
		break;
	case seek_state::end:
		// Figure out the size of the file below.
		break;
	}
	
	stream_size_type readOffset;
	stream_size_type blockNumber = m_p->block_number(offset());
	compressor_thread_lock l(m_p->compressor());
	if (size() % m_p->m_blockItems == 0)
		readOffset = m_p->current_file_size(l);
	else if (blockNumber == m_p->m_streamBlocks)
		readOffset = m_p->current_file_size(l);
	else if (blockNumber == m_p->m_streamBlocks - 1)
		readOffset = m_p->last_block_read_offset(l);
	else {
		tp_assert(false, "get_position: Invalid block_number");
		readOffset = 1111111111111111111ull; // avoid compiler warning
	}
	return stream_position(readOffset, offset());
}

void compressed_stream_base::set_position(const stream_position & pos) {
	m_p->m_updateReadOffsetFromWrite = false;
	
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
	
	if (!m_p->use_compression() && pos.read_offset() != 0)
		throw stream_exception("set_position: Invalid position, read_offset != 0");
	
	if (pos.offset() > size())
		throw stream_exception("set_position: Invalid position, offset > size");
	
	if (m_p->m_buffer.get() != 0
		&& m_p->block_number(pos.offset()) == m_p->buffer_block_number())
	{
		if (pos.read_offset() != m_p->m_readOffset) {
			// We don't always know the read offset of the current block
			// in m_readOffset, so let's assume that
			// pos.read_offset() is correct.
		}
		
		m_p->m_readOffset = pos.read_offset();
		m_offset = pos.offset();
		m_nextItem = m_bufferBegin + m_p->m_itemSize * m_p->block_item_index();
		m_seekState = seek_state::none;
		return;
	}
	
	m_p->m_nextPosition = pos;
	m_seekState = seek_state::position;
	m_p->uncache_read_writes();
}

bool compressed_stream_base::is_readable() const noexcept { return m_p->m_canRead; }

bool compressed_stream_base::is_writable() const noexcept { return m_p->m_canWrite; }

void compressed_stream_base::open(const std::string & path,
								  access_type accessType,
								  memory_size_type userDataSize,
								  cache_hint cacheHint,
								  compression_flags compressionFlags) {
	open(path, translate(accessType, cacheHint, compressionFlags), userDataSize);
}

void compressed_stream_base::open(memory_size_type userDataSize,
								  cache_hint cacheHint,
								  compression_flags compressionFlags) {
	open(translate(access_read_write, cacheHint, compressionFlags), userDataSize);
}

void compressed_stream_base::open(temp_file & file,
								  access_type accessType,
								  memory_size_type userDataSize,
								  cache_hint cacheHint,
								  compression_flags compressionFlags) {
	open(file, translate(accessType, cacheHint, compressionFlags), userDataSize);
}

void compressed_stream_base::open(const std::string & path, compression_flags compressionFlags) {
	const memory_size_type userDataSize = 0;
	open(path, translate(access_read_write, access_sequential, compressionFlags), userDataSize);
}

void compressed_stream_base::open(compression_flags compressionFlags) {
	const memory_size_type userDataSize = 0;
	open(translate(access_read_write, access_sequential, compressionFlags), userDataSize);
}

void compressed_stream_base::open(temp_file & file, compression_flags compressionFlags) {
	const memory_size_type userDataSize = 0;
	open(file, translate(access_read_write, access_sequential, compressionFlags), userDataSize);
}

bool compressed_stream_base::is_open() const noexcept { return m_p->m_open; }

stream_size_type compressed_stream_base::offset() const {
	switch (m_seekState) {
	case seek_state::none:
		return m_offset;
	case seek_state::beginning:
		return 0;
	case seek_state::end:
		return size();
	case seek_state::position:
		return m_p->m_nextPosition.offset();
	}
	tp_assert(false, "offset: Unreachable statement; m_seekState invalid");
	return 0; // suppress compiler warning
}

std::string compressed_stream_base::describe() {
	std::stringstream ss;
	describe(ss);
	return ss.str();
}


} // namespace tpie
