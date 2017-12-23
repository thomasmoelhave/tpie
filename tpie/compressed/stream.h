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
/// \file compressed/stream.h  Compressed stream public API.
///////////////////////////////////////////////////////////////////////////////

#include <tpie/array.h>
#include <tpie/tpie_assert.h>
#include <tpie/tempname.h>
#include <tpie/file_base_crtp.h>
#include <tpie/file_stream_base.h>
#include <tpie/file_accessor/byte_stream_accessor.h>
#include <tpie/compressed/thread.h>
#include <tpie/compressed/buffer.h>
#include <tpie/compressed/request.h>
#include <tpie/compressed/stream_position.h>
#include <tpie/compressed/direction.h>
#include <tpie/stream_writable.h>

namespace tpie {

struct open {
	enum type {
		/** Open a file for reading only. */
		read_only =  00000001,
		/** Open a file for writing only.
		 * Content is truncated. */
		write_only = 00000002,
		/** Neither sequential access nor random access is intended.
		 * Corresponds to POSIX_FADV_NORMAL. */
		access_normal = 00000004,
		/** Random access is intended.
		 * Corresponds to POSIX_FADV_RANDOM and FILE_FLAG_RANDOM_ACCESS (Win32). */
		access_random = 00000010,
		/** Compress some blocks
		 * according to available resources (time, memory). */
		compression_normal = 00000020,
		/** Compress all blocks according to the preferred compression scheme
		 * which can be set using
		 * tpie::the_compressor_thread().set_preferred_compression(). */
		compression_all = 00000040,

		defaults = 0
	};

	friend inline open::type operator|(open::type a, open::type b)
	{ return (open::type) ((int) a | (int) b); }
	friend inline open::type operator&(open::type a, open::type b)
	{ return (open::type) ((int) a & (int) b); }
	friend inline open::type operator^(open::type a, open::type b)
	{ return (open::type) ((int) a ^ (int) b); }
	friend inline open::type operator~(open::type a)
	{ return (open::type) ~(int) a; }

	static type translate(access_type accessType, cache_hint cacheHint, compression_flags compressionFlags) {
		return (type) ((

			(accessType == access_read) ? read_only :
			(accessType == access_write) ? write_only :
			defaults) | (

			(cacheHint == tpie::access_normal) ? access_normal :
			(cacheHint == tpie::access_random) ? access_random :
			defaults) | (

			(compressionFlags == tpie::compression_normal) ? compression_normal :
			(compressionFlags == tpie::compression_all) ? compression_all :
			defaults));
	}

	static cache_hint translate_cache(open::type openFlags) {
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

	static compression_flags translate_compression(open::type openFlags) {
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
};


///////////////////////////////////////////////////////////////////////////////
/// \brief  Base class containing the implementation details that are
/// independent of the item type.
///////////////////////////////////////////////////////////////////////////////
class compressed_stream_base {
public:
	typedef std::shared_ptr<compressor_buffer> buffer_t;

	static const file_stream_base::offset_type beginning = file_stream_base::beginning;
	static const file_stream_base::offset_type end = file_stream_base::end;
	static const file_stream_base::offset_type current = file_stream_base::current;

	typedef file_stream_base::offset_type offset_type;
protected:
	struct seek_state {
		enum type {
			none,
			beginning,
			end,
			position
		};
	};

	compressed_stream_base(memory_size_type itemSize,
						   double blockFactor);

	// Non-virtual, protected destructor
	~compressed_stream_base();

	void open_inner(const std::string & path,
					open::type openFlags,
					memory_size_type userDataSize);

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
		if (sizeof(TT) != user_data_size())
			throw stream_exception("Wrong user data size");
		read_user_data(reinterpret_cast<void *>(&data), sizeof(TT));
	}

	memory_size_type read_user_data(void * data, memory_size_type count);

	template <typename TT>
	void write_user_data(const TT & data) {
		if (sizeof(TT) > max_user_data_size())
			throw stream_exception("Wrong user data size");
		write_user_data(reinterpret_cast<const void *>(&data), sizeof(TT));
	}

	void write_user_data(const void * data, memory_size_type count);

	memory_size_type user_data_size() const;

	memory_size_type max_user_data_size() const;

	const std::string & path() const;

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Deprecated interface for opening a named stream.
	///
	/// If compressionFlags is compression_none and the file does not already
	/// exist, no compression will be used when writing.
	/// If compressionFlags is compression_normal and the file does not already
	/// exist, compression will be used when writing.
	/// If the file already exists, the compression flags of the existing file
	/// are used instead.
	///////////////////////////////////////////////////////////////////////////
	void open(const std::string & path,
			  access_type accessType,
			  memory_size_type userDataSize = 0,
			  cache_hint cacheHint=access_sequential,
			  compression_flags compressionFlags=compression_none)
	{
		open(path, open::translate(accessType, cacheHint, compressionFlags), userDataSize);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Deprecated interface for opening an unnamed temporary stream.
	///////////////////////////////////////////////////////////////////////////
	void open(memory_size_type userDataSize,
			  cache_hint cacheHint=access_sequential,
			  compression_flags compressionFlags=compression_none) {
		open(open::translate(access_read_write, cacheHint, compressionFlags), userDataSize);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Deprecated interface for opening a temporary stream.
	///////////////////////////////////////////////////////////////////////////
	void open(temp_file & file,
			  access_type accessType,
			  memory_size_type userDataSize = 0,
			  cache_hint cacheHint=access_sequential,
			  compression_flags compressionFlags=compression_none) {
		open(file, open::translate(accessType, cacheHint, compressionFlags), userDataSize);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Deprecated interface for opening a named stream.
	///////////////////////////////////////////////////////////////////////////
	void open(const std::string & path, compression_flags compressionFlags) {
		const memory_size_type userDataSize = 0;
		open(path, open::translate(access_read_write, access_sequential, compressionFlags), userDataSize);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Deprecated interface for opening an unnamed temporary stream.
	///////////////////////////////////////////////////////////////////////////
	void open(compression_flags compressionFlags) {
		const memory_size_type userDataSize = 0;
		open(open::translate(access_read_write, access_sequential, compressionFlags), userDataSize);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Deprecated interface for opening a temporary stream.
	///////////////////////////////////////////////////////////////////////////
	void open(temp_file & file, compression_flags compressionFlags) {
		const memory_size_type userDataSize = 0;
		open(file, open::translate(access_read_write, access_sequential, compressionFlags), userDataSize);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Open and possibly create a stream.
	///
	/// The stream is created if it does not exist and opened for reading
	/// and writing, but this can be changed with open::read_only or
	/// open::write_only; see below.
	///
	/// The flags supplied to openFlags should be a combination of the
	/// following from \c open::type, OR'ed together:
	///
	/// open::read_only
	///     Open for reading only, and fail if the stream does not exist.
	///
	/// open::write_only
	///     Open for writing only, and truncate the stream if it exists.
	///
	/// open::access_normal
	///     By default, POSIX_FADV_SEQUENTIAL is passed to the open syscall
	///     to indicate that the OS should optimize for sequential access;
	///     this flag disables that flag.
	///
	/// open::access_random
	///	    Pass POSIX_FADV_RANDOM to the open syscall to make the OS optimize
	///	    for random access.
	///
	/// open::compression_normal
	///     Create the stream in compression mode if it does not already exist,
	///     and compress written blocks according to available resources (for
	///     instance CPU time and memory).
	///
	/// open::compression_all
	///     Create the stream in compression mode if it does not already exist,
	///     and compress all written blocks using the preferred compression
	///     scheme, which can be set using
	///     tpie::the_compressor_thread().set_preferred_compression().
	///
	/// \param path  The path to the file to open
	/// \param openFlags  A bit-wise combination of the flags; see above.
	/// \param userDataSize  Required user data capacity in stream header.
	///////////////////////////////////////////////////////////////////////////
	void open(const std::string & path, open::type openFlags=open::defaults, memory_size_type userDataSize=0);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Open and create an unnamed temporary stream.
	///
	/// \param openFlags  A bit-wise combination of the flags; see above.
	/// \param userDataSize  Required user data capacity in stream header.
	///////////////////////////////////////////////////////////////////////////
	void open(open::type openFlags=open::defaults, memory_size_type userDataSize=0);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Open and possibly create a temporary stream.
	///
	/// \param file  The temporary file to open
	/// \param openFlags  A bit-wise combination of the flags; see above.
	/// \param userDataSize  Required user data capacity in stream header.
	///////////////////////////////////////////////////////////////////////////
	void open(temp_file & file, open::type openFlags=open::defaults, memory_size_type userDataSize=0);

	void close();

protected:
	void finish_requests(compressor_thread_lock & l);

	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	///
	/// Precondition: use_compression()
	///
	/// TODO: Should probably investigate when this reports a useful value.
	///////////////////////////////////////////////////////////////////////////
	stream_size_type last_block_read_offset(compressor_thread_lock & l);

	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	///
	/// Precondition: use_compression()
	///
	/// TODO: Should probably investigate when this reports a useful value.
	///////////////////////////////////////////////////////////////////////////
	stream_size_type current_file_size(compressor_thread_lock & l);

	bool use_compression() { return m_byteStreamAccessor.get_compressed(); }

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Reset cheap read/write counts to zero so that the next
	/// read/write operation will check stream state properly.
	///////////////////////////////////////////////////////////////////////////
	void uncache_read_writes() {
		m_cachedReads = m_cachedWrites = 0;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Truncate to zero size.
	///////////////////////////////////////////////////////////////////////////
	void truncate_zero();

	void truncate_uncompressed(stream_size_type offset);

	void truncate_compressed(const stream_position & pos);
	
	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	///
	/// Precondition: seekState != none
	///
	/// Sets seekState to none.
	///
	/// If anything fails, the stream is closed by a close_on_fail_guard.
	///////////////////////////////////////////////////////////////////////////
	void perform_seek(read_direction::type dir=read_direction::forward);
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Gets buffer for given block and sets bufferBegin and bufferEnd,
	/// and sets bufferDirty to false.
	///////////////////////////////////////////////////////////////////////////
	void get_buffer(compressor_thread_lock & l, stream_size_type blockNumber); 

	///////////////////////////////////////////////////////////////////////////
	/// Blocks to take the compressor lock.
	///
	/// Precondition: m_bufferDirty == true.
	/// Postcondition: m_bufferDirty == false.
	///
	/// Does not get a new block buffer.
	///////////////////////////////////////////////////////////////////////////
	void flush_block(compressor_thread_lock & lock);

	void maybe_update_read_offset(compressor_thread_lock & lock);

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Reads next block according to nextReadOffset/nextBlockSize.
	///
	/// Updates m_readOffset with the new read offset.
	///////////////////////////////////////////////////////////////////////////
	void read_next_block(compressor_thread_lock & lock, stream_size_type blockNumber);

	void read_previous_block(compressor_thread_lock & lock, stream_size_type blockNumber);


	void read_block(compressor_thread_lock & lock,
					stream_size_type readOffset,
					read_direction::type readDirection);


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
		return block_number(m_offset);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute the number of the block currently loaded into m_buffer.
	///
	/// Precondition: m_buffer.get() != 0.
	/// Precondition: m_seekState == none
	///////////////////////////////////////////////////////////////////////////
	stream_size_type buffer_block_number() {
		stream_size_type blockNumber = block_number();
		if (m_nextItem == m_bufferEnd)
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
		return block_item_index(m_offset);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Compute number of cheap, unchecked reads/writes we can do from
	/// now.
	///////////////////////////////////////////////////////////////////////////
	void cache_read_writes();

	void peak_unlikely();
	
	void read_back_unlikely();
	
	void write_unlikely(const char * item);
	
public:
	bool is_open() const { return m_open; }

	stream_size_type size() const { return m_size; }

	stream_size_type file_size() const { return size(); }

	stream_size_type offset() const {
		switch (m_seekState) {
			case seek_state::none:
				return m_offset;
			case seek_state::beginning:
				return 0;
			case seek_state::end:
				return size();
			case seek_state::position:
				return m_nextPosition.offset();
		}
		tp_assert(false, "offset: Unreachable statement; m_seekState invalid");
		return 0; // suppress compiler warning
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  For debugging: Describe the internal stream state in a string.
	///////////////////////////////////////////////////////////////////////////
	void describe(std::ostream & out);
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief  For debugging: Describe the internal stream state in a string.
	///////////////////////////////////////////////////////////////////////////
	std::string describe() {
		std::stringstream ss;
		describe(ss);
		return ss.str();
	}

	void post_open() {
		seek(0);
	}

	///////////////////////////////////////////////////////////////////////////
	/// Precondition: is_open()
	/// Precondition: offset == 0
	///////////////////////////////////////////////////////////////////////////
	void seek(stream_offset_type offset, offset_type whence=beginning);
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Truncate to given size.
	///
	/// Precondition: compression is disabled or offset is size() or 0.
	/// Blocks to take the compressor lock.
	///////////////////////////////////////////////////////////////////////////
	void truncate(stream_size_type offset);
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Truncate to given stream position.
	///////////////////////////////////////////////////////////////////////////
	void truncate(const stream_position & pos);
	
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
	stream_position get_position();
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Seek to a position that was previously recalled with
	/// \c get_position.
	///////////////////////////////////////////////////////////////////////////
	void set_position(const stream_position & pos);
	
	///////////////////////////////////////////////////////////////////////////
	/// \brief  Check if the next call to read() will succeed or not.
	///////////////////////////////////////////////////////////////////////////
	bool can_read() {
		if (m_cachedReads > 0)
			return true;

		if (!this->m_open)
			return false;

		return offset() < size();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief  Check if the next call to read_back() will succeed or not.
	///////////////////////////////////////////////////////////////////////////
	bool can_read_back() {
		if (!this->m_open)
			return false;

		return offset() > 0;
	}
protected:
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
	/** Number of cheap, unchecked reads we can do next. */
	memory_size_type m_cachedReads;
	/** Number of cheap, unchecked writes we can do next. */
	memory_size_type m_cachedWrites;
	/** The anonymous temporary file we have opened (when appropriate). */
	tpie::unique_ptr<temp_file> m_ownedTempFile;
	/** The temporary file we have opened (when appropriate).
	 * When m_ownedTempFile.get() != 0, m_tempFile == m_ownedTempFile.get(). */
	temp_file * m_tempFile;
	/** File accessor. */
	file_accessor::byte_stream_accessor<default_raw_file_accessor> m_byteStreamAccessor;
	/** Number of logical items in the stream. */
	stream_size_type m_size;
	/** Buffer manager for this entire stream. */
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

	seek_state::type m_seekState;

	/** Position relating to the currently loaded buffer.
	 * readOffset is only valid during reading.
	 * Invariants:
	 *
	 * If use_compression() == false, readOffset == 0.
	 * If offset == 0, then readOffset == block_item_index() == block_number() == 0.
	 */
	stream_size_type m_readOffset;

	/** Offset of next item to read/write, relative to beginning of stream.
	 * Invariants:
	 *
	 * block_number() in [0, m_streamBlocks]
	 * offset in [0, size]
	 * block_item_index() in [0, m_blockSize)
	 * offset == block_number() * m_blockItems + block_item_index()
	 *
	 * block_item_index() <= offset.
	 *
	 * If block_number() == m_streamBlocks, we are in a block that has not yet
	 * been written to disk.
	 */
	stream_size_type m_offset;

	/** If seekState is `position`, seek to this position before reading/writing. */
	stream_position m_nextPosition;

	stream_size_type m_nextReadOffset;

	/** Only when m_buffer.get() != 0: First item in writable buffer. */
	char * m_bufferBegin;
	/** Only when m_buffer.get() != 0: End of writable buffer. */
	char * m_bufferEnd;

	/** Next item in buffer to read/write. */
	char * m_nextItem;
};



///////////////////////////////////////////////////////////////////////////////
/// \brief  Compressed stream.
///
/// We assume that `T` is trivially copyable and that its copy constructor
/// and assignment operator never throws.
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
class file_stream : public compressed_stream_base {
	static_assert(is_stream_writable<T>::value, "file_stream item type must be trivially copyable");
public:
	typedef T item_type;
	
	file_stream(double blockFactor=1.0)
		: compressed_stream_base(sizeof(T), blockFactor) {}
	
	~file_stream() {
		try {
			close();
		} catch (std::exception & e) {
			log_error() << "Someone threw an error in file_stream::~file_stream: " << e.what() << std::endl;
			abort();
		}
	}

	static constexpr memory_size_type memory_usage(double blockFactor=1.0) noexcept {
		// m_buffer is included in m_buffers memory usage
		return sizeof(file_stream)
			+ sizeof(temp_file) // m_ownedTempFile
			+ stream_buffers::memory_usage(block_size(blockFactor)) // m_buffers;
			;
	}
public:
	///////////////////////////////////////////////////////////////////////////
	/// Reads next item from stream if can_read() == true.
	///
	/// If can_read() == false, throws an end_of_stream_exception.
	///
	/// Blocks to take the compressor lock.
	///
	/// If a stream_exception is thrown, the stream is left in the state it was
	/// in before the call to read().
	///////////////////////////////////////////////////////////////////////////
	const T & read() {
		if (m_cachedReads == 0) {
			peak_unlikely();
			const T & res = *reinterpret_cast<const T*>(m_nextItem);
			++m_offset;
			m_nextItem += sizeof(T);
			cache_read_writes();
			return res;
		}
		--m_cachedReads;
		++m_offset;
		const T & res = *reinterpret_cast<const T*>(m_nextItem);
		m_nextItem += sizeof(T);
		return res;
	}
	
	///////////////////////////////////////////////////////////////////////////
	/// Peeks next item from stream if can_read() == true.
	///
	/// If can_read() == false, throws an end_of_stream_exception.
	///
	/// Blocks to take the compressor lock.
	///
	/// If a stream_exception is thrown, the stream is left in the state it was
	/// in before the call to peek().
	///////////////////////////////////////////////////////////////////////////
	const T & peek() {
		if (m_cachedReads == 0) peak_unlikely();
		return *reinterpret_cast<const T*>(m_nextItem);
	}

	void skip() {
		read();
	}

	void skip_back() {
		read_back();
	}

	///////////////////////////////////////////////////////////////////////////
	/// Precondition: is_open().
	///
	/// Reads min(b-a, size()-offset()) items into the range [a, b).
	/// If less than b-a items are read, throws an end_of_stream_exception.
	///////////////////////////////////////////////////////////////////////////
	template <typename IT>
	void read(IT const a, IT const b) {
		for (IT i = a; i != b; ++i) *i = read();
	}
	
	const T & read_back() {
		if (m_seekState != seek_state::none || m_nextItem == m_bufferBegin) read_back_unlikely();
		++m_cachedReads;
		--m_offset;
		m_nextItem -= sizeof(T);
		return *reinterpret_cast<const T *>(m_nextItem);
	}
	
	void write(const T & item) {
		if (m_cachedWrites == 0) {
			write_unlikely(reinterpret_cast<const char*>(&item));
			return;
		}
		memcpy(m_nextItem, &item, sizeof(T));
		m_nextItem += sizeof(T);
		++m_size;
		++m_offset;
		--m_cachedWrites;
		return;
	}

	template <typename IT>
	void write(IT const a, IT const b) {
		for (IT i = a; i != b; ++i) write(*i);
	}

private:
};

} // namespace tpie

#endif // TPIE_COMPRESSED_STREAM_H
