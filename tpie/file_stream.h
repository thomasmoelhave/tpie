// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, 2011, The TPIE development team
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
#ifndef __TPIE_FILE_STREAM_H__
#define __TPIE_FILE_STREAM_H__
#include <tpie/tempname.h>
#include <tpie/file.h>
#include <tpie/memory.h>
////////////////////////////////////////////////////////////////////////////
/// \file file_stream.h
/// \brief Implement simple class aggregating both a file and a file::stream
////////////////////////////////////////////////////////////////////////////

namespace tpie {

////////////////////////////////////////////////////////////////////////////
/// \brief Simple class aggregating both as file and a file::stream
///
/// A file stream basicly supports every operation a file or
/// a file::stream stream supports. This is used to access a file
/// io-efficiently, and is the direct replacement of the old
/// ami::stream
///
/// \tparam T The type of items stored in the stream
////////////////////////////////////////////////////////////////////////////
template <typename T>
class file_stream {
public:
	/** The type of the items stored in the stream */
	typedef T item_type;

	/////////////////////////////////////////////////////////////////////////
	/// \brief Construct a new file_stream
	/// 
	/// \param blockFactor The relative size of a block compared to the 
	/// default
	/// \param fileAccessor The file accessor to use, if none is supplied a
	/// default will be used
	/////////////////////////////////////////////////////////////////////////
	inline file_stream(double blockFactor=1.0, 
					   file_accessor::file_accessor * fileAccessor=NULL)
		throw()
		// : m_file(blockFactor, fileAccessor), m_stream(m_file, 0)
		{

		m_size = 0;
		m_itemSize = sizeof(item_type);
		m_open = false;
		if (fileAccessor == 0)
			fileAccessor = new default_file_accessor();
		m_fileAccessor = fileAccessor;

		m_blockItems = block_size(blockFactor)/m_itemSize;

		m_blockStartIndex = 0;
		m_nextBlock = std::numeric_limits<stream_size_type>::max();
		m_nextIndex = std::numeric_limits<memory_size_type>::max();
		m_index = std::numeric_limits<memory_size_type>::max();

		m_block.data = tpie_new_array<char>(block_memory_usage());
		m_block.size = 0;
		m_block.number = std::numeric_limits<stream_size_type>::max();
		m_block.dirty = false;
	};

	inline ~file_stream() {
		close();
		tpie_delete_array(m_block.data, block_memory_usage());
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::open
	/// \sa file_base::open
	/////////////////////////////////////////////////////////////////////////
	inline void open(const std::string & path,
					 file_base::access_type accessType=file_base::read_write,
					 memory_size_type user_data_size=0) throw (stream_exception) {
		close();
		m_canRead = accessType == file_base::read || accessType == file_base::read_write;
		m_canWrite = accessType == file_base::write || accessType == file_base::read_write;
		m_fileAccessor->open(path, m_canRead, m_canWrite, m_itemSize, user_data_size);
		m_size = m_fileAccessor->size();
		m_open = true;

		initialize();
		seek(0);
	}


	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::read_user_data
	/// \sa file_base::read_user_data
	/////////////////////////////////////////////////////////////////////////
	template <typename TT>
	void read_user_data(TT & data) throw (stream_exception) {
		assert(m_open);
		if (sizeof(TT) != m_fileAccessor->user_data_size()) throw io_exception("Wrong user data size");
		m_fileAccessor->read_user_data(reinterpret_cast<void*>(&data));
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::write_user_data
	/// \sa file_base::write_user_data
	/////////////////////////////////////////////////////////////////////////
	template <typename TT>
	void write_user_data(const TT & data) {
		assert(m_open);
		if (sizeof(TT) != m_fileAccessor->user_data_size()) throw io_exception("Wrong user data size");
		m_fileAccessor->write_user_data(reinterpret_cast<const void*>(&data));
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Close the fileand release resources
	///
	/// This will close the file and resources used by buffers and such
	/////////////////////////////////////////////////////////////////////////
	inline void close() throw(stream_exception) {
		if (m_open) {
			flush_block();
			m_fileAccessor->close();
		}
		m_open = false;
	}
	
	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file<T>::stream::write(const item_type & item)
	/// \sa file<T>::stream::write(const item_type & item)
	/////////////////////////////////////////////////////////////////////////
	inline void write(const item_type & item) throw(stream_exception) {
		assert(m_open);
#ifndef NDEBUG
		if (!is_writable())
			throw io_exception("Cannot write to read only stream");
#endif
		if (m_index >= m_blockItems) update_block();
		reinterpret_cast<T*>(m_block.data)[m_index++] = item;
		write_update();
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file<T>::stream::write(const IT & start, const IT & end)
	/// \sa file<T>::stream::write(const IT & start, const IT & end)
	/////////////////////////////////////////////////////////////////////////
	template <typename IT>
	inline void write(const IT & start, const IT & end) throw(stream_exception) {
		assert(m_open);
		IT i = start;
		while (i != end) {
			if (m_index >= m_blockItems) update_block();

			IT blockmax = i + (m_blockItems-m_index);

			T * dest = reinterpret_cast<T*>(m_block.data) + m_index;

			IT till = std::min(end, blockmax);

			std::copy(i, till, dest);

			m_index += till - i;
			write_update();
			i = till;
		}
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file<T>::stream::read()
	/// \sa file<T>::stream::read()
	/////////////////////////////////////////////////////////////////////////
	inline const item_type & read() throw(stream_exception) {
		assert(m_open);
		if (m_index >= m_block.size) {
			update_block();
			if (offset() >= size()) {
				throw end_of_stream_exception();
			}
		}
		return reinterpret_cast<T*>(m_block.data)[m_index++];
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file<T>::stream::read(const IT & start, const IT & end)
	/// \sa file<T>::stream::read(const IT & start, const IT & end)
	/////////////////////////////////////////////////////////////////////////
	template <typename IT>
	inline void read(const IT & start, const IT & end) throw(stream_exception) {
		assert(m_open);
		IT i = start;
		while (i != end) {
			if (m_index >= m_blockItems) {
				// check to make sure we have enough items in the stream
				stream_size_type offs = offset();
				if (offs >= size()
					|| offs + (end-i) >= size()) {

					throw end_of_stream_exception();
				}

				// fetch next block from disk
				update_block();
			}

			T * src = reinterpret_cast<T*>(m_block.data) + m_index;

			// either read the rest of the block or until `end'
			memory_size_type count = std::min(m_blockItems-m_index, static_cast<memory_size_type>(end-i));

			std::copy(src, src + count, i);

			// advance output iterator
			i += count;

			// advance input position
			m_index += count;
		}
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file<T>::stream::read_back()
	/// \sa file<T>::stream::read_back()
	/////////////////////////////////////////////////////////////////////////
	inline const item_type & read_back() throw(stream_exception) {
		assert(m_open);
		seek(-1, file_base::current);
		const item_type & i = read();
		seek(-1, file_base::current);
		return i;
	}


	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file<T>::stream::read(const IT & start, const IT & end)
	/// \sa file<T>::stream::read(const IT & start, const IT & end)
	/////////////////////////////////////////////////////////////////////////
	/* TODO
	template <typename IT>
	inline void read_back(const IT & start, const IT & end) throw(stream_exception) {
		m_stream.read(start, end);
	}
	*/

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::stream::offset()
	/// \sa file_base::stream::offset()
	/////////////////////////////////////////////////////////////////////////
	inline stream_size_type offset() const throw() {
		assert(m_open);
		if (m_nextBlock == std::numeric_limits<stream_size_type>::max())
			return m_index + m_blockStartIndex;
		return m_nextIndex + m_nextBlock * m_blockItems;
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::path()
	/// \sa file_base::path()
	/////////////////////////////////////////////////////////////////////////
	inline const std::string & path() const throw() {
		assert(m_open);
		return m_fileAccessor->path();
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::size()
	/// \sa file_base::size()
	/////////////////////////////////////////////////////////////////////////
	inline stream_size_type size() const throw() {
		// XXX update_vars changes internal state in a way that is not visible
		// through the class interface.
		// therefore, a const_cast is warranted.
		const_cast<file_stream<T>*>(this)->update_vars();
		return m_size;
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::stream::can_read()
	/// \sa file_base::stream::can_read()
	/////////////////////////////////////////////////////////////////////////
	inline bool can_read() const throw() {
		assert(m_open);
		if (m_index < m_block.size) return true;
		return offset() < m_size;
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::stream::can_read_back()
	/// \sa file_base::stream::can_read_back()
	/////////////////////////////////////////////////////////////////////////
	inline bool can_read_back() const throw() {
		assert(m_open);
		const_cast<file_stream<T>*>(this)->update_vars();
		if (m_index <= m_block.size) return true;
		if (m_nextBlock == std::numeric_limits<stream_size_type>::max())
			return m_block.number != 0;
		else
			return true;
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::stream::seek()
	/// \sa file_base::stream::seek()
	/////////////////////////////////////////////////////////////////////////
	inline void seek(stream_offset_type offset, 
					 file_base::offset_type whence=file_base::beginning) 
		throw (stream_exception) {

		assert(m_open);
		if (whence == file_base::end)
			offset += size();
		else if (whence == file_base::current) {
			// are we seeking into the current block?
			if (offset >= 0 || static_cast<stream_size_type>(-offset) <= m_index) {
				stream_size_type new_index = static_cast<stream_offset_type>(offset+m_index);

				if (new_index < m_blockItems) {
					update_vars();
					m_index = new_index;
					return;
				}
			}

			offset += this->offset();
		}
		if (0 > offset || (stream_size_type)offset > size())
			throw io_exception("Tried to seek out of file");
		update_vars();
		stream_size_type b = static_cast<stream_size_type>(offset) / m_blockItems;
		m_index = static_cast<memory_size_type>(offset - b*m_blockItems);
		if (b == m_block.number) {
			m_nextBlock = std::numeric_limits<stream_size_type>::max();
			m_nextIndex = std::numeric_limits<memory_size_type>::max();
			assert(this->offset() == (stream_size_type)offset);
			return;
		}
		m_nextBlock = b;
		m_nextIndex = m_index;
		m_index = std::numeric_limits<memory_size_type>::max();
		assert(this->offset() == (stream_size_type)offset);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::truncate()
	/// \sa file_base::truecate()
	///
	/// Note that when using a file_stream the stream will automaticly be
	/// seeked back if it is beond the new end of the file. 
	/////////////////////////////////////////////////////////////////////////
	inline void truncate(stream_size_type size) {
		stream_size_type o=offset();
		//TODO flush current block here
		m_size = size;
		m_fileAccessor->truncate(size);
		seek(std::min(o, size));
	}

	////////////////////////////////////////////////////////////////////////////////
	/// Check if we can read from the file
	///
	/// \returns True if we can read from the file
	////////////////////////////////////////////////////////////////////////////////
	bool is_readable() const throw() {
		return m_canRead;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// Check if we can write to the file
	///
	/// \returns True if we can write to the file
	////////////////////////////////////////////////////////////////////////////////
	bool is_writable() const throw() {
		return m_canWrite;
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Calculate the amount of memory used by count file_streams
	///
	/// \param count The number of file_streams, to concider.
	/// \param blockFactor The block factor you promice to pass to open
	/// \param includeDefaultFileAccessor Unless you are supplieng your own
	/// file accessor to open leave this to be true
	/// \returns The amount of memory maximaly used by the count file_streams
	/////////////////////////////////////////////////////////////////////////
	inline static memory_size_type memory_usage(
		float blockFactor=1.0,
		bool includeDefaultFileAccessor=true) throw() {
		// TODO
		memory_size_type x = sizeof(file_stream);
		x += block_memory_usage(blockFactor); // allocated in constructor
		if (includeDefaultFileAccessor)
			x += default_file_accessor::memory_usage();
		return x;
	}

private:
	struct block_t {
		memory_size_type size;
		stream_size_type number;
		bool dirty;
		char *data;
	};

	memory_size_type m_index;
	stream_size_type m_nextBlock;
	memory_size_type m_nextIndex;
	stream_size_type m_blockStartIndex;
	memory_size_type m_blockItems;
	stream_size_type m_size;
	bool m_canRead;
	bool m_canWrite;
	memory_size_type m_itemSize;
	bool m_open;
	file_accessor::file_accessor * m_fileAccessor;

	block_t m_block;

	/**
	 * Use file_accessor to fetch indicated block no. into m_block
	 */
	inline void get_block(stream_size_type block) {
		// If the file contains n full blocks (numbered 0 through n-1), we may
		// request any block in {0, 1, ... n}

		// If the file contains n-1 full blocks and a single non-full block, we may
		// request any block in {0, 1, ... n-1}

		// We capture this restraint with the assertion:
		assert(block * static_cast<stream_size_type>(m_blockItems) <= size());

		m_block.dirty = false;
		m_block.number = block;

		// calculate buffer size
		m_block.size = m_blockItems;
		if (static_cast<stream_size_type>(m_block.size) + m_block.number * static_cast<stream_size_type>(m_blockItems) > size())
			m_block.size = size() - m_block.number * m_blockItems;

		// populate buffer data
		if (m_block.size > 0 &&
			m_fileAccessor->read(m_block.data, m_block.number * static_cast<stream_size_type>(m_blockItems), m_block.size) != m_block.size) {
			throw io_exception("Incorrect number of items read");
		}
	}

	/**
	 * Fetch block from disk as indicated by m_nextBlock, writing old block to
	 * disk if needed.
	 */
	inline void update_block() {
		if (m_nextBlock == std::numeric_limits<stream_size_type>::max()) {
			m_nextBlock = m_block.number+1;
			m_nextIndex = 0;
		}
		flush_block();
		get_block(m_nextBlock);
		m_blockStartIndex = m_nextBlock*static_cast<stream_size_type>(m_blockItems);
		m_index = m_nextIndex;
		m_nextBlock = std::numeric_limits<stream_size_type>::max();
		m_nextIndex = std::numeric_limits<memory_size_type>::max();
	}

	/**
	 * Write block to disk.
	 */
	inline void flush_block() {
		if (m_block.dirty || !m_canRead) {
			assert(m_canWrite);
			update_vars();
			m_fileAccessor->write(m_block.data, m_block.number * static_cast<stream_size_type>(m_blockItems), m_block.size);
		}
	}

	inline void update_vars() {
		if (m_block.dirty && m_index != std::numeric_limits<memory_size_type>::max()) {
			assert(m_index <= m_blockItems);
			m_block.size = std::max(m_block.size, m_index);
			m_size = std::max(m_size, static_cast<stream_size_type>(m_index)+m_blockStartIndex);
		}
	}

	inline void initialize() {
		flush_block();
		m_nextBlock = std::numeric_limits<stream_size_type>::max();
		m_nextIndex = std::numeric_limits<memory_size_type>::max();
		m_index = std::numeric_limits<memory_size_type>::max();
	}

	inline void write_update() {
		m_block.dirty = true;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// Calculate the block size in bytes used by a stream.
	///
	/// \param blockFactor Factor of the global block size to use
	/// \returns Size in Bytes
	////////////////////////////////////////////////////////////////////////////////
	static inline memory_size_type block_size(double blockFactor) throw () {
		return static_cast<memory_size_type>(2 * 1024*1024 * blockFactor);
	}

	static inline double calculate_block_factor(memory_size_type blockSize) throw () {
		return (double)blockSize / (double)block_size(1.0);
	}

	static inline memory_size_type block_memory_usage(double blockFactor) {
		return block_size(blockFactor);
	}

	inline memory_size_type block_memory_usage() {
		return m_blockItems * m_itemSize;
	}
};
}

#endif //__TPIE_FILE_STREAM_H__
