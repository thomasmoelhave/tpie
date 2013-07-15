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

#ifndef TPIE_COMPRESSED_STREAM_H
#define TPIE_COMPRESSED_STREAM_H

///////////////////////////////////////////////////////////////////////////////
/// \file compressed/stream.h
///////////////////////////////////////////////////////////////////////////////

#include <snappy.h>
#include <tpie/array.h>
#include <tpie/tempname.h>
#include <tpie/file_base_crtp.h>
#include <tpie/file_stream_base.h>
#include <tpie/file_accessor/byte_stream_accessor.h>
#include <tpie/compressed/thread.h>
#include <tpie/compressed/buffer.h>
#include <tpie/compressed/request.h>
#include <tpie/compressed/position.h>

namespace tpie {

class compressed_stream_base {
public:
	typedef boost::shared_ptr<compressor_buffer> buffer_t;

protected:
	struct seek_state {
		enum type {
			none,
			beginning,
			end,
			position
		};
	};

	struct buffer_state {
		enum type {
			write_only,
			read_only
		};
	};

	compressed_stream_base(memory_size_type itemSize,
						   double blockFactor);

	// Non-virtual, protected destructor
	~compressed_stream_base();

	virtual void flush_block() = 0;

	virtual void post_open() = 0;

	void open_inner(const std::string & path,
					access_type accessType,
					memory_size_type userDataSize,
					cache_hint cacheHint);

	compressor_thread & compressor() { return the_compressor_thread(); }

public:
	bool is_readable() const throw() { return m_canRead; }

	bool is_writable() const throw() { return m_canWrite; }

	static memory_size_type block_size(double blockFactor) throw ();

	static double calculate_block_factor(memory_size_type blockSize) throw ();

	static memory_size_type block_memory_usage(double blockFactor);

	memory_size_type block_items() const;

	memory_size_type block_size() const;

	template <typename TT>
	void read_user_data(TT & data) {
		read_user_data(reinterpret_cast<void *>(&data), sizeof(TT));
	}

	memory_size_type read_user_data(void * data, memory_size_type count);

	template <typename TT>
	void write_user_data(const TT & data) {
		write_user_data(reinterpret_cast<const void *>(&data), sizeof(TT));
	}

	void write_user_data(const void * data, memory_size_type count);

	memory_size_type user_data_size() const;

	memory_size_type max_user_data_size() const;

	const std::string & path() const;

	void open(const std::string & path,
			  access_type accessType = access_read_write,
			  memory_size_type userDataSize = 0,
			  cache_hint cacheHint=access_sequential);

	void open(memory_size_type userDataSize = 0,
			  cache_hint cacheHint = access_sequential);

	void open(temp_file & file,
			  access_type accessType = access_read_write,
			  memory_size_type userDataSize = 0,
			  cache_hint cacheHint = access_sequential);

	void close();

protected:
	///////////////////////////////////////////////////////////////////////////
	/// Exception guarantee: nothrow
	///////////////////////////////////////////////////////////////////////////
	void finish_requests(compressor_thread_lock & l);

	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	///
	/// TODO: Should probably investigate when this reports a useful value.
	///
	/// Exception guarantee: nothrow
	///////////////////////////////////////////////////////////////////////////
	stream_size_type last_block_read_offset(compressor_thread_lock & l);

	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	///
	/// TODO: Should probably investigate when this reports a useful value.
	///
	/// Exception guarantee: nothrow
	///////////////////////////////////////////////////////////////////////////
	stream_size_type current_file_size(compressor_thread_lock & l);

public:
	bool is_open() const { return m_open; }

	stream_size_type size() const { return m_size; }

	stream_size_type file_size() const { return size(); }

	stream_size_type offset() const {
		switch (m_seekState) {
			case seek_state::none:
				return m_position.offset();
			case seek_state::beginning:
				return 0;
			case seek_state::end:
				return size();
			case seek_state::position:
				return m_nextPosition.offset();
		}
		throw exception("offset: Unreachable statement; m_seekState invalid");
	}

protected:
	/** Whether the current block must be written out to disk before being ejected.
	 * Invariants:
	 * If m_bufferDirty is true, block_number() is either
	 * m_streamBlocks or m_streamBlocks - 1.
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
	/** File accessor. */
	file_accessor::byte_stream_accessor<default_raw_file_accessor> m_byteStreamAccessor;
	/** The anonymous temporary file we have opened (when appropriate). */
	tpie::auto_ptr<temp_file> m_ownedTempFile;
	/** The temporary file we have opened (when appropriate).
	 * When m_ownedTempFile.get() != 0, m_tempFile == m_ownedTempFile.get(). */
	temp_file * m_tempFile;
	/** Number of logical items in the stream. */
	stream_size_type m_size;
	/** Buffer holding the items of the block currently being read/written. */
	buffer_t m_buffer;
	/** Buffer manager for this entire stream. */
	stream_buffers m_buffers;

	/** The number of blocks written to the file. */
	stream_size_type m_streamBlocks;

	/** Read offset of the last block in the stream.
	 * Necessary to support seeking to the end. */
	stream_size_type m_lastBlockReadOffset;

	/** Response from compressor thread; protected by compressor thread mutex. */
	compressor_response m_response;

	/** Whenever seekState is set to none,
	 * bufferState must be set appropriately. */
	seek_state::type m_seekState;

	/** Whether m_buffer is read-only or write-only.
	 * When seekState is not none, bufferState has nothing to do
	 * with offset()/get_position()!
	 * After reading the final item of the stream, bufferState will remain
	 * read-only, but can_read() == false and write() succeeds.
	 */
	buffer_state::type m_bufferState;

	/** Position relating to the currently loaded buffer.
	 * Only valid during reading.
	 * Invariants:
	 *
	 * block_number() in [0, m_streamBlocks]
	 * offset in [0, size]
	 * block_item_index() in [0, m_blockSize)
	 * offset == block_number() * m_blockItems + block_item_index()
	 *
	 * If offset == 0, then read_offset == block_item_index() == block_number() == 0.
	 * If block_number() != 0, then read_offset, offset != 0.
	 * block_item_index() <= offset.
	 *
	 * If block_number() == m_streamBlocks, we are in a block that has not yet
	 * been written to disk.
	 */
	stream_position m_position;

	/** If seekState is `position`, seek to this position before reading/writing. */
	stream_position m_nextPosition;

	/** If nextBlockSize is zero,
	 * the size of the block to read is the first eight bytes,
	 * and the block begins after those eight bytes.
	 * If nextBlockSize is non-zero,
	 * the next block begins at the given offset and has the given size.
	 */
	stream_size_type m_nextReadOffset;
	stream_size_type m_nextBlockSize;
};

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

namespace ami {
	template <typename T> class cstream;
}

///////////////////////////////////////////////////////////////////////////////
/// \brief  Compressed stream.
///
/// We assume that `T` is trivially copyable and that its copy constructor
/// and assignment operator never throws.
///
/// Exception safety for each method is documented as either
/// `basic` (no leaks; invariants upheld),
/// `strong` (transaction semantics), or
/// `nothrow` (no exception thrown).
///
/// As a rule of thumb, when a `tpie::stream_exception` is thrown from a method,
/// the stream is left in the state it was in prior to the method call.
/// When a `tpie::exception` is thrown, the stream may have changed.
/// In particular, the stream may have been closed, and it is up to the caller
/// (if the exception is caught) to ensure that the stream is reopened as
/// necessary.
///
/// Several methods claim the `nothrow` guarantee even though the
/// implementation has `throw` statements.
/// In this case, there are two reasons an exception may be thrown:
/// A `tpie::exception` is thrown if some invariant in the stream has been
/// violated, and this is a bug we must fix in the compressed stream.
/// A `tpie::stream_exception` is thrown if the user has violated a
/// precondition (for instance by passing an invalid parameter).
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class compressed_stream : public compressed_stream_base {
	using compressed_stream_base::seek_state;
	using compressed_stream_base::buffer_state;

	friend class ami::cstream<T>;

	static const file_stream_base::offset_type beginning = file_stream_base::beginning;
	static const file_stream_base::offset_type end = file_stream_base::end;
	static const file_stream_base::offset_type current = file_stream_base::current;

public:
	typedef T item_type;
	typedef file_stream_base::offset_type offset_type;

	compressed_stream(double blockFactor=1.0)
		: compressed_stream_base(sizeof(T), blockFactor)
		, m_bufferBegin(0)
		, m_bufferEnd(0)
		, m_nextItem(0)
		, m_lastItem(0)
	{
	}

	~compressed_stream() {
		try {
			close();
		} catch (std::exception & e) {
			log_error() << "Someone threw an error in compressed_stream::~compressed_stream: " << e.what() << std::endl;
			throw;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  For debugging: Describe the internal stream state in a string.
	///
	/// Exception guarantee: nothrow
	///////////////////////////////////////////////////////////////////////////
	void describe(std::ostream & out) {
		if (!this->is_open()) {
			out << "[Closed stream]";
			return;
		}

		out << "[(" << m_byteStreamAccessor.path() << ") item " << offset()
			<< " of " << size();
		out << " (block " << block_number()
			<< " @ byte " << m_position.read_offset()
			<< ", item " << block_item_index()
			<< ")";

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
				out << " (block " << block_number(m_nextPosition)
					<< " @ byte " << m_nextPosition.read_offset()
					<< ", item " << block_item_index(m_nextPosition)
					<< ")";
				break;
		}

		switch (m_bufferState) {
			case buffer_state::write_only:
				out << ", buffer write-only";
				break;
			case buffer_state::read_only:
				out << ", buffer read-only";
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

		out << ']';
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  For debugging: Describe the internal stream state in a string.
	///
	/// Exception guarantee: nothrow
	///////////////////////////////////////////////////////////////////////////
	std::string describe() {
		std::stringstream ss;
		describe(ss);
		return ss.str();
	}

	virtual void post_open() override {
		seek(0);
	}

	///////////////////////////////////////////////////////////////////////////
	/// Precondition: is_open()
	/// Precondition: offset == 0
	/// Exception guarantee: nothrow
	///////////////////////////////////////////////////////////////////////////
	void seek(stream_offset_type offset, offset_type whence=beginning) {
		if (!is_open()) throw stream_exception("seek: !is_open");
		if (offset != 0) throw stream_exception("Random seeks are not supported");
		switch (whence) {
		case beginning:
			if (m_buffer.get() != 0 && buffer_block_number() == 0) {
				// We are already reading or writing the first block.
				m_lastItem = m_nextItem;
				m_nextItem = m_bufferBegin;
				m_position = stream_position(0, 0);
				m_bufferState = (size() > 0) ? buffer_state::read_only : buffer_state::write_only;
				m_seekState = seek_state::none;
			} else {
				// We need to load the first block on the next I/O.
				m_seekState = seek_state::beginning;
			}
			return;
		case end:
			if (m_buffer.get() == 0) {
				m_seekState = seek_state::end;
			} else if (m_bufferState == buffer_state::write_only) {
				// no-op
				m_seekState = seek_state::none;
				m_bufferState = buffer_state::write_only;
			} else {
				log_debug() << "Block number " << block_number()
					<< ", stream blocks " << m_streamBlocks << std::endl;
				if (block_number() == m_streamBlocks) {
					m_nextItem = m_lastItem;
					m_position = stream_position(m_position.read_offset(), size());
				} else {
					m_seekState = seek_state::end;
				}
			}
			return;
		case current:
			return;
		}
		throw stream_exception("seek: Unknown whence");
	}

	///////////////////////////////////////////////////////////////////////////
	/// Precondition: offset is size() or 0.
	/// Blocks to take the compressor lock.
	/// Exception guarantee: nothrow
	///////////////////////////////////////////////////////////////////////////
	void truncate(stream_size_type offset) {
		if (offset == size()) return;
		if (offset != 0)
			throw stream_exception("Arbitrary truncate is not supported");

		// No need to flush block
		m_buffer.reset();
		compressor_thread_lock l(compressor());
		finish_requests(l);
		get_buffer(l, 0);
		m_size = 0;
		m_streamBlocks = 0;
		m_bufferState = buffer_state::write_only;
		m_byteStreamAccessor.truncate(0);

		// Since block_number == 0, seek(0) should short circuit
		// and not do any I/O.
		seek(0);
		if (m_seekState != seek_state::none)
			throw exception("Unexpected seek state in truncate");
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	/// Exception guarantee: nothrow
	///////////////////////////////////////////////////////////////////////////
	stream_position get_position() {
		switch (m_seekState) {
			case seek_state::position:
				// We just set_position, so we can just return what we got.
				return m_nextPosition;
			case seek_state::beginning:
				return stream_position(0, 0);
			case seek_state::none:
				if (m_bufferState == buffer_state::read_only) {
					if (m_nextItem == m_bufferEnd) {
						stream_size_type readOffset = m_nextReadOffset;
						if (m_nextBlockSize != 0)
							readOffset -= sizeof(m_nextBlockSize);
						return stream_position(readOffset, offset());
					}
					return m_position;
				} else {
					// write-only
					if (m_nextItem == m_bufferEnd) {
						if (!m_bufferDirty)
							throw exception("Write-only at end of buffer, but bufferDirty is false?");
						// Make sure the position we get is not at the end of a block
						flush_block();
						m_nextItem = m_bufferBegin;
					}
				}
				// Else, our buffer is write-only, and we are not seeking,
				// meaning the write head is at the end of the stream
				// (since we do not support overwriting).
				break;
			case seek_state::end:
				// Figure out the size of the file below.
				break;
		}

		stream_size_type readOffset;
		compressor_thread_lock l(compressor());
		if (size() % m_blockItems == 0)
			readOffset = current_file_size(l);
		else if (block_number() == m_streamBlocks)
			readOffset = current_file_size(l);
		else
			readOffset = last_block_read_offset(l);
		return stream_position(readOffset, size());
	}

	///////////////////////////////////////////////////////////////////////////
	/// Exception guarantee: nothrow
	///////////////////////////////////////////////////////////////////////////
	void set_position(const stream_position & pos) {
		// If the code is correct, short circuiting is not necessary;
		// if the code is not correct, short circuiting might mask faults.
		/*
		if (pos == m_position) {
			m_seekState = seek_state::none;
			m_bufferState = something?
			return;
		}
		*/

		if (m_buffer.get() != 0
			&& block_number(pos) == buffer_block_number())
		{
			if (pos.read_offset() != m_position.read_offset()) {
				// We don't always know the read offset of the current block
				// in m_position.read_offset(), so let's assume that
				// pos.read_offset() is correct.
			}

			m_position = pos;
			if (offset() == size()) {
				m_bufferState = buffer_state::write_only;
			} else {
				if (m_bufferState == buffer_state::write_only) {
					// record size of current block
					m_lastItem = m_nextItem;
				}
				m_bufferState = buffer_state::read_only;
			}
			m_nextItem = m_bufferBegin + block_item_index();
			m_seekState = seek_state::none;
			return;
		}

		// This short-circuiting will delay an I/O on the next can_read(),
		// but probably does not improve performance in practice.
		/*
		if (pos.read_offset() == 0 && pos.offset() == 0) {
			m_seekState = seek_state::beginning;
			m_bufferState = something?
			return;
		}
		*/

		m_nextPosition = pos;
		m_seekState = seek_state::position;
	}

private:
	///////////////////////////////////////////////////////////////////////////
	/// Reads next item from stream if can_read() == true.
	///
	/// If can_read() == false, throws an end_of_stream_exception.
	///
	/// Blocks to take the compressor lock.
	///
	/// Exception guarantee: strong
	///////////////////////////////////////////////////////////////////////////
	const T & read_ref() {
		if (!can_read()) throw end_of_stream_exception();
		if (m_seekState != seek_state::none) perform_seek();
		if (m_nextItem == m_lastItem) {
			if (m_nextItem != m_bufferEnd)
				throw exception("read: end of block, can_read(), but block is not full");
			compressor_thread_lock l(compressor());
			// At this point, block_number() == buffer_block_number() + 1
			read_next_block(l, block_number());
		}
		if (m_nextItem == m_bufferEnd) {
			log_debug() << "m_bufferEnd == " << m_bufferEnd
				<< ", m_lastItem == " << m_lastItem << std::endl;
			throw exception("m_nextItem reached m_bufferEnd before m_lastItem");
		}
		m_position.advance_item();
		return *m_nextItem++;
	}

public:
	///////////////////////////////////////////////////////////////////////////
	/// \copybrief read_ref()
	/// \copydetails read_ref()
	///////////////////////////////////////////////////////////////////////////
	T read() {
		return read_ref();
	}

	///////////////////////////////////////////////////////////////////////////
	/// Precondition: is_open().
	///
	/// Reads min(b-a, size()-offset()) items into the range [a, b).
	/// If less than b-a items are read, throws an end_of_stream_exception.
	///
	/// Exception guarantee: basic.
	///////////////////////////////////////////////////////////////////////////
	template <typename IT>
	void read(IT const a, IT const b) {
		for (IT i = a; i != b; ++i) *i = read();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Check if the next call to read() will succeed or not.
	///
	/// Exception guarantee: nothrow
	///////////////////////////////////////////////////////////////////////////
	bool can_read() {
		if (!this->m_open)
			return false;

		return offset() < size();
	}

	///////////////////////////////////////////////////////////////////////////
	/// Precondition: is_open() && !can_read().
	///
	/// Exception guarantee:
	/// nothrow if seekState == none.
	/// basic otherwise.
	///////////////////////////////////////////////////////////////////////////
	void write(const T & item) {
		if (m_seekState != seek_state::none) perform_seek();

		if (offset() != size())
			throw stream_exception("Non-appending write attempted");

		if (m_bufferState == buffer_state::read_only && offset() == size()) {
			m_bufferState = buffer_state::write_only;
		}

		if (m_nextItem == m_bufferEnd) {
			if (m_bufferDirty) {
				flush_block();
			} else {
				compressor_thread_lock l(compressor());
				get_buffer(l, m_streamBlocks);
			}
			m_nextItem = m_bufferBegin;
		}

		*m_nextItem++ = item;
		this->m_bufferDirty = true;
		++m_size;
		m_position.advance_item();
	}

	///////////////////////////////////////////////////////////////////////////
	/// Precondition: is_open() && !can_read().
	///
	/// Exception guarantee: basic.
	///////////////////////////////////////////////////////////////////////////
	template <typename IT>
	void write(IT const a, IT const b) {
		for (IT i = a; i != b; ++i) write(*i);
	}

private:
	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	///
	/// Precondition: seekState != none
	///
	/// Sets seekState to none.
	///
	/// If anything fails, the stream is closed by a close_on_fail_guard.
	///
	/// Exception guarantee:
	/// seekState == beginning: nothrow
	/// seekState == position: strong (closes the file)
	/// seekState == end: nothrow (see TODO below)
	///////////////////////////////////////////////////////////////////////////
	void perform_seek() {
		// This must be initialized before the compressor lock below,
		// so that it is destructed after we free the lock.
		close_on_fail_guard closeOnFail(this);

		if (m_seekState == seek_state::none)
			throw exception("perform_seek when seekState is none");

		// For debugging: Verify get_position()
		stream_position claimedPosition = get_position();

		if (this->m_bufferDirty) {
			flush_block();
		}

		m_buffer.reset();
		compressor_thread_lock l(compressor());
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
			if (size() == 0)
				throw exception("Seek beginning when size is zero");
			m_nextReadOffset = 0;
			read_next_block(l, 0);
			m_bufferState = buffer_state::read_only;
			m_position = stream_position(0, 0);
		} else if (m_seekState == seek_state::position) {
			m_nextReadOffset = m_nextPosition.read_offset();
			m_nextBlockSize = 0;
			stream_size_type blockNumber = block_number(m_nextPosition);
			memory_size_type blockItemIndex = block_item_index(m_nextPosition);

			// This cannot happen in practice due to the implementation of
			// block_number and block_item_index, but it is an important
			// assumption in the following code.
			if (blockItemIndex >= m_blockItems)
				throw exception("perform_seek: Computed block item index >= blockItems");

			// We have previously ensured we will end up in a read-only state,
			// so this method will not throw an end_of_stream_exception().
			try {
				read_next_block(l, blockNumber);
			} catch (end_of_stream_exception &) {
				throw stream_exception("perform_seek: Seek to invalid position (got end_of_stream)");
			}

			memory_size_type blockItems = m_lastItem - m_bufferBegin;

			if (blockItemIndex > blockItems) {
				throw stream_exception("perform_seek: Item offset out of bounds");
			} else if (blockItemIndex == blockItems) {
				// We cannot end up at the end of the stream,
				// so this block must be non-full.
				throw exception("perform_seek: Non-full block in the middle of the stream");
			}
			m_nextItem = m_bufferBegin + blockItemIndex;
			m_position = m_nextPosition;
			m_bufferState = buffer_state::read_only;
		} else if (m_seekState == seek_state::end) {
			if (m_streamBlocks * m_blockItems == size()) {
				// The last block in the stream is full,
				// so we can safely start a new empty one.
				get_buffer(l, m_streamBlocks);
				m_bufferState = buffer_state::write_only;
				m_nextItem = m_bufferBegin;
				// We don't care about m_position.read_offset().
				// Set read offset to roughly 0.9637 * 2^60 bytes,
				// a value that should be easy to spot in a debugger,
				// and one that should not otherwise occur in practice.
				// It's a prime number, too!
				m_position = stream_position(1111111111111111111ull, size());
			} else {
				// The last block in the stream is non-full.
				if (m_streamBlocks == 0) {
					// TODO: I don't think this can happen in practice,
					// since we short-circuit seek(end) when streamBlocks == 0.
					throw exception("Attempted seek to end when no blocks have been written");
				}
				stream_size_type readOffset = last_block_read_offset(l);
				m_nextReadOffset = readOffset;
				m_nextBlockSize = 0;
				read_next_block(l, m_streamBlocks - 1);
				m_nextItem = m_lastItem;
				m_bufferState = buffer_state::write_only;
				m_position = stream_position(readOffset, size());
			}
		} else {
			log_debug() << "Unknown seek state " << m_seekState << std::endl;
			throw exception("perform_seek: Unknown seek state");
		}

		m_seekState = seek_state::none;

		l.get_lock().unlock();
		// For debugging: Verify get_position()
		if (claimedPosition != get_position())
			throw exception("get_position() was changed by perform_seek().");

		closeOnFail.commit();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Gets buffer for given block and sets bufferBegin and bufferEnd,
	/// and sets bufferDirty to false.
	///
	/// Exception guarantee: nothrow
	///////////////////////////////////////////////////////////////////////////
	void get_buffer(compressor_thread_lock & l, stream_size_type blockNumber) {
		m_buffer = this->m_buffers.get_buffer(l, blockNumber);
		m_bufferBegin = reinterpret_cast<T *>(m_buffer->get());
		m_bufferEnd = m_bufferBegin + block_items();
		this->m_bufferDirty = false;
	}

	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	///
	/// Precondition: m_bufferDirty == true.
	///
	/// Sets bufferDirty to false and gets the buffer for the next block.
	///
	/// Exception guarantee: nothrow
	///////////////////////////////////////////////////////////////////////////
	virtual void flush_block() override {
		compressor_thread_lock lock(compressor());

		stream_size_type blockNumber = buffer_block_number();
		stream_size_type truncateTo;
		if (blockNumber == m_streamBlocks) {
			// New block; no truncate
			truncateTo = std::numeric_limits<stream_size_type>::max();
			++m_streamBlocks;
		} else if (blockNumber == m_streamBlocks - 1) {
			// Block rewrite; truncate
			truncateTo = last_block_read_offset(lock);
			m_response.clear_block_info();
		} else {
			throw exception("flush_block: blockNumber not at end of stream");
		}

		m_lastBlockReadOffset = std::numeric_limits<stream_size_type>::max();

		if (m_nextItem == NULL) throw exception("m_nextItem is NULL");
		if (m_bufferBegin == NULL) throw exception("m_bufferBegin is NULL");
		memory_size_type blockItems = m_nextItem - m_bufferBegin;
		m_buffer->set_size(blockItems * sizeof(T));
		compressor_request r;
		r.set_write_request(m_buffer,
							&m_byteStreamAccessor,
							truncateTo,
							blockItems,
							blockNumber,
							&m_response);
		compressor().request(r);
		get_buffer(lock, m_streamBlocks);
		// get_buffer sets bufferDirty to false.
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Reads next block according to nextReadOffset/nextBlockSize.
	///
	/// Throws end_of_stream_exception on end of stream.
	/// Updates m_position with the new read offset.
	///
	/// Exception guarantee: nothrow
	///////////////////////////////////////////////////////////////////////////
	void read_next_block(compressor_thread_lock & lock, stream_size_type blockNumber) {
		get_buffer(lock, blockNumber);

		compressor_request r;
		r.set_read_request(m_buffer,
						   &m_byteStreamAccessor,
						   m_nextReadOffset,
						   m_nextBlockSize,
						   &m_response);

		compressor().request(r);
		while (!m_response.done()) {
			m_response.wait(lock);
		}
		if (m_response.end_of_stream())
			throw end_of_stream_exception();

		if (blockNumber >= m_streamBlocks)
			m_streamBlocks = blockNumber + 1;

		stream_size_type readOffset = m_nextReadOffset;
		if (m_nextBlockSize != 0) readOffset -= sizeof(readOffset);

		m_position = stream_position(readOffset, m_position.offset());

		m_nextReadOffset = m_response.next_read_offset();
		m_nextBlockSize = m_response.next_block_size();
		m_nextItem = m_bufferBegin;
		memory_size_type itemsRead = m_buffer->size() / sizeof(T);
		m_lastItem = m_bufferBegin + itemsRead;
		m_bufferState = buffer_state::read_only;
	}

	stream_size_type block_number(const stream_position & position) {
		return position.offset() / m_blockItems;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute the number of the block containing the next
	/// read/written item.
	///
	/// Precondition: m_buffer.get() != 0.
	///////////////////////////////////////////////////////////////////////////
	stream_size_type block_number() {
		return block_number(m_position);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute the number of the block currently loaded into m_buffer.
	///
	/// Precondition: m_buffer.get() != 0.
	///////////////////////////////////////////////////////////////////////////
	stream_size_type buffer_block_number() {
		stream_size_type blockNumber = block_number();
		if (m_nextItem == m_bufferEnd)
			return blockNumber - 1;
		else
			return blockNumber;
	}

	memory_size_type block_item_index(const stream_position & position) {
		stream_size_type i = position.offset() % m_blockItems;
		memory_size_type cast = static_cast<memory_size_type>(i);
		if (i != cast)
			throw exception("Block item index out of bounds");
		return cast;
	}

	memory_size_type block_item_index() {
		return block_item_index(m_position);
	}

private:
	/** Only when m_buffer.get() != 0: First item in writable buffer. */
	T * m_bufferBegin;
	/** Only when m_buffer.get() != 0: End of writable buffer. */
	T * m_bufferEnd;

	/** Next item in buffer to read/write. */
	T * m_nextItem;
	/** Only used in read mode: End of readable buffer. */
	T * m_lastItem;
};

} // namespace tpie

#endif // TPIE_COMPRESSED_STREAM_H
