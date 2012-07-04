// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, 2011, 2012 The TPIE development team
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
#ifndef __TPIE_FILE_BASE_H__
#define __TPIE_FILE_BASE_H__
#include <tpie/exception.h>
#include <tpie/file_accessor/file_accessor.h>
#ifndef WIN32
#include <tpie/file_accessor/posix.h>
#else ////WIN32
#include <tpie/file_accessor/win32.h>
#endif //WIN32
#include <boost/intrusive/list.hpp>
#include <tpie/tempname.h>
#include <memory>
#include <tpie/memory.h>

#ifndef WIN32
typedef tpie::file_accessor::stream_accessor<tpie::file_accessor::posix> default_file_accessor;
#else //WIN32
typedef tpie::file_accessor::stream_accessor<tpie::file_accessor::win32> default_file_accessor;
#endif //WIN32

namespace tpie {

class file_base_base {
public:
  	/** Type describing how we should interpret the offset supplied to seek. */
	enum offset_type {
		beginning,
		end,
		current
	};

	/** Type describing how we wish to access a file. */
	enum access_type {
		read,
		write,
		read_write
	};


	////////////////////////////////////////////////////////////////////////////////
	/// Check if we can read from the file.
	///
	/// \returns True if we can read from the file.
	////////////////////////////////////////////////////////////////////////////////
	bool is_readable() const throw() {
		return m_canRead;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// Check if we can write to the file.
	///
	/// \returns True if we can write to the file.
	////////////////////////////////////////////////////////////////////////////////
	bool is_writable() const throw() {
		return m_canWrite;
	}


	////////////////////////////////////////////////////////////////////////////////
	/// Calculate the block size in bytes used by a stream.
	///
	/// We have block_size(calculate_block_factor(b)) ~= b.
	///
	/// \param blockFactor Factor of the global block size to use.
	/// \returns Size in bytes.
	////////////////////////////////////////////////////////////////////////////////
	static inline memory_size_type block_size(double blockFactor) throw () {
		return static_cast<memory_size_type>(2 * 1024*1024 * blockFactor);
	}

	///////////////////////////////////////////////////////////////////////////
	/// Find the block factor that would result in the given block size
	/// measured in bytes.
	///
	/// We have calculate_block_factor(block_size(f)) ~= f.
	///
	/// \param blockSize The sought block size.
	/// \returns The block factor needed to achieve this block size.
	///////////////////////////////////////////////////////////////////////////
	static inline double calculate_block_factor(memory_size_type blockSize) throw () {
		return (double)blockSize / (double)block_size(1.0);
	}

	static inline memory_size_type block_memory_usage(double blockFactor) {
		return block_size(blockFactor);
	}

	inline memory_size_type block_memory_usage() {
		return m_blockItems * m_itemSize;
	}


	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the number of items per block.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type blockItems() {
		return m_blockItems;
	}

	memory_size_type blockSize() {
		return m_blockSize;
	}


	/////////////////////////////////////////////////////////////////////////
	/// \brief Read the user data associated with the file.
	///
	/// \param data Where to store the user data.
	/// \tparam TT The type of user data. sizeof(TT) must be equal to the
	/// user_data_size supplied to the open call.
	/////////////////////////////////////////////////////////////////////////
	template <typename TT>
	void read_user_data(TT & data) throw(stream_exception) {
		assert(m_open);
		if (sizeof(TT) != m_fileAccessor->user_data_size()) throw io_exception("Wrong user data size");
		m_fileAccessor->read_user_data(reinterpret_cast<void*>(&data));
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Write user data to the stream.
	///
	/// \param data The user data to store in the stream.
	/// \tparam TT The type of user data. sizeof(TT) must be equal to the
	/// user_data_size supplied to the open call.
	/////////////////////////////////////////////////////////////////////////
	template <typename TT>
	void write_user_data(const TT & data) throw(stream_exception) {
		assert(m_open);
		if (sizeof(TT) != m_fileAccessor->user_data_size()) throw io_exception("Wrong user data size");
		m_fileAccessor->write_user_data(reinterpret_cast<const void*>(&data));
	}


	/////////////////////////////////////////////////////////////////////////
	/// \brief The path of the file opened or the empty string.
	///
	/// \returns The path of the currently opened file.
	/////////////////////////////////////////////////////////////////////////
	inline const std::string & path() const throw() {
		assert(m_open);
		return m_fileAccessor->path();
	}

	inline void open_inner(const std::string & path,
						   access_type accessType=read_write,
						   memory_size_type userDataSize=0) throw(stream_exception) {
		m_canRead = accessType == read || accessType == read_write;
		m_canWrite = accessType == write || accessType == read_write;
		m_fileAccessor->open(path, m_canRead, m_canWrite, m_itemSize, m_blockSize, userDataSize);
		m_size = m_fileAccessor->size();
		m_open = true;
	}
public:
	memory_size_type m_blockItems;
	memory_size_type m_blockSize;
	bool m_canRead;
	bool m_canWrite;
	bool m_open;
	memory_size_type m_itemSize;
	file_accessor::file_accessor * m_fileAccessor;
	tpie::auto_ptr<temp_file> m_ownedTempFile;
	temp_file * m_tempFile;
	stream_size_type m_size;
};

template <typename child_t>
class file_base_base_crtp: public file_base_base  {
public:
	child_t & self() {return *static_cast<child_t *>(this);}
	const child_t & self() const {return *static_cast<child_t *>(this);}


	/////////////////////////////////////////////////////////////////////////
	/// \brief Open a file.
	///
	/// \param path The path of the file to open.
	/// \param accessType The mode of operation.
	/// \param userDataSize The size of the user data we want to store in the
	/// file.
	/////////////////////////////////////////////////////////////////////////
	inline void open(const std::string & path,
					 access_type accessType=read_write,
					 memory_size_type userDataSize=0) throw (stream_exception) {
		self().close();
		self().open_inner(path, accessType, userDataSize);
	}

	/////////////////////////////////////////////////////////////////////////
	///
	/////////////////////////////////////////////////////////////////////////
	inline void open(memory_size_type userDataSize=0) throw (stream_exception) {
		self().close();
		m_ownedTempFile.reset(new temp_file());
		m_tempFile=m_ownedTempFile.get();
		self().open_inner(m_tempFile->path(), read_write, userDataSize);
	}

	inline void open(temp_file & file, 
					 access_type accessType=read_write,
					 memory_size_type userDataSize=0) throw (stream_exception) {
		self().close();
		m_tempFile=&file;
		self().open_inner(m_tempFile->path(), accessType, userDataSize);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Close the file.
	///
	/// Note all streams into the file must be freed before you call close.
	/////////////////////////////////////////////////////////////////////////
	inline void close() throw(stream_exception) {
		if (m_open) m_fileAccessor->close();
		m_open = false;
		m_tempFile = NULL;
		m_ownedTempFile.reset();
	}
};


class file_base: public file_base_base_crtp<file_base> {
protected:
	///////////////////////////////////////////////////////////////////////////
	/// This is the type of our block buffers. We have one per file::stream
	/// distributed over two linked lists.
	///////////////////////////////////////////////////////////////////////////
	struct block_t : public boost::intrusive::list_base_hook<> {
		memory_size_type size;
		memory_size_type usage;
		stream_size_type number;
		bool dirty;
		char data[0];
	};
public:


	/////////////////////////////////////////////////////////////////////////
	/// \brief Calculate the size of the file measured in items.
	///
	/// \returns The number of items is the file.
	/////////////////////////////////////////////////////////////////////////
	inline stream_size_type size() const throw() {
		return m_size;
	}


	inline void update_size(stream_size_type size) {
		m_size = std::max(m_size, size);
		if (m_tempFile) 
			m_tempFile->update_recorded_size(m_fileAccessor->byte_size());
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Stream in file. We support multiple streams per file.
	///////////////////////////////////////////////////////////////////////////
	class stream {
	protected:
		/** Associated file object. */
		file_base & m_file;
		/** Item index into the current block, or maxint if we don't have a
		 * block. */
		memory_size_type m_index;
		/** After a cross-block seek: Block index of next block, or maxint if
		 * the current block is good enough OR if we haven't read/written
		 * anything yet. */
		stream_size_type m_nextBlock;
		/** After a cross-block seek: Item index into next block. Otherwise,
		 * maxint as with m_nextBlock. */
		memory_size_type m_nextIndex;
		/** The file-level item index of the first item in the current block.
		 * When m_block is not the null block, this should be equal to
		 * m_block->number * block_items(). */
		stream_size_type m_blockStartIndex;
		/** Current block. May be equal to &m_file.m_emptyBlock to indicate no
		 * current block. */
		block_t * m_block;

		///////////////////////////////////////////////////////////////////////
		/// Update m_block, m_index, m_nextBlock and m_nextIndex. If
		/// m_nextBlock is maxint, use next block is the one numbered
		/// m_block->number+1. m_index is updated with the value of
		/// m_nextIndex.
		///////////////////////////////////////////////////////////////////////
		void update_block();

		///////////////////////////////////////////////////////////////////////
		/// Fetch number of items per block.
		///////////////////////////////////////////////////////////////////////
		inline memory_size_type block_items() const {return m_file.m_blockItems;}
		
		// this turns out to be slower than cmp+cmov, so we use std::max in
		// write_update instead.
		//static inline int64_t max(int64_t x, int64_t y) {
		//    return x-(((x-y)>>63)&(x-y));
		//}

		///////////////////////////////////////////////////////////////////////
		/// Call whenever the current block buffer is modified. Since we
		/// support multiple streams per block, we must always keep
		/// m_block->size updated when m_block is the trailing block (or the
		/// only block) in the file. For the same reasons we keep m_file.m_size
		/// updated.
		///////////////////////////////////////////////////////////////////////
		inline void write_update() {
			m_block->dirty = true;
			// with optimization, each of these std::max is compiled on an x86
			// into cmp (compare), cmov (conditional move).
			// TODO: with inline assembly we could do a single comparisons and two
			// cmovs, as the two comparison results will always be the same.
			m_block->size = std::max(m_block->size, m_index);
			m_file.update_size(static_cast<stream_size_type>(m_index)+m_blockStartIndex);
		}
	public:
		
		///////////////////////////////////////////////////////////////////////
		/// \brief Create a stream associated with the given file.
		/// \param file The file to associate with this stream.
		/// \param offset The file-level item offset to seek to.
		///////////////////////////////////////////////////////////////////////
		stream(file_base & file, stream_size_type offset=0);

		///////////////////////////////////////////////////////////////////////
		/// \brief Free the current block buffer, flushing it to the disk.
		///////////////////////////////////////////////////////////////////////
		void free();

		inline ~stream() {free();}

		/////////////////////////////////////////////////////////////////////////
		/// \brief Moves the logical offset in the stream.
		///
		/// \param offset Where to move the logical offset to.
		/// \param whence Move the offset relative to what.
		/////////////////////////////////////////////////////////////////////////
		inline void seek(stream_offset_type offset, offset_type whence=beginning) throw(stream_exception) {
			assert(m_file.m_open);
			if (whence == end)
				offset += size();
			else if (whence == current) {
				// are we seeking into the current block?
				if (offset >= 0 || static_cast<stream_size_type>(-offset) <= m_index) {
					stream_size_type new_index = static_cast<stream_offset_type>(offset+m_index);

					if (new_index < m_file.m_blockItems) {
						m_index = static_cast<memory_size_type>(new_index);
						return;
					}
				}

				offset += this->offset();
			}
			if (0 > offset || (stream_size_type)offset > size())
				throw io_exception("Tried to seek out of file");
			stream_size_type b = static_cast<stream_size_type>(offset) / m_file.m_blockItems;
			m_index = static_cast<memory_size_type>(offset - b*m_file.m_blockItems);
			if (b == m_block->number) {
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

		///////////////////////////////////////////////////////////////////////
		/// \brief Get the size of the file underlying this stream.
		///////////////////////////////////////////////////////////////////////
 		inline stream_size_type size() const throw() {
			assert(m_file.m_open);
			return m_file.size();
		}

		/////////////////////////////////////////////////////////////////////////
		/// \brief Calculate the current offset in the stream.
		///
		/// \returns The current offset in the stream
		/////////////////////////////////////////////////////////////////////////
 		inline stream_size_type offset() const throw() {
			assert(m_file.m_open);
 			if (m_nextBlock == std::numeric_limits<stream_size_type>::max())
 				return m_index + m_blockStartIndex;
 			return m_nextIndex + m_nextBlock * m_file.m_blockItems;
 		}

		/////////////////////////////////////////////////////////////////////////
		/// \brief Check if we can read an item with read().
		///
		/// This is logically equivalent to:
		/// \code
		/// return offset() < size();
		/// \endcode
		/// but it might be faster.
		///
		/// \returns Whether or not we can read more items from the stream.
		/////////////////////////////////////////////////////////////////////////
 		inline bool can_read() const throw() {
			assert(m_file.m_open);
 			if (m_index < m_block->size) return true;
 			return offset() < size();
 		}

		/////////////////////////////////////////////////////////////////////////
		/// \brief Check if we can read an item with read_back().
		///
		/// \returns Whether or not we can read an item with read_back().
		/////////////////////////////////////////////////////////////////////////
		inline bool can_read_back() const throw() {
			assert(m_file.m_open);
 			if (m_index <= m_block->size) return true;
			if (m_nextBlock == std::numeric_limits<stream_size_type>::max())
				return m_block->number != 0;
			else
				return true;
		}

		///////////////////////////////////////////////////////////////////////
		/// \brief Set up block buffers and offsets.
		///////////////////////////////////////////////////////////////////////
		inline void initialize() {
			if (m_block != &m_file.m_emptyBlock) m_file.free_block(m_block);
			m_nextBlock = std::numeric_limits<stream_size_type>::max();
			m_nextIndex = std::numeric_limits<memory_size_type>::max();
			m_index = std::numeric_limits<memory_size_type>::max();
			m_block = &m_file.m_emptyBlock;
		}
	};

	///////////////////////////////////////////////////////////////////////////
	/// \brief Truncate file to given size.
	///////////////////////////////////////////////////////////////////////////
	void truncate(stream_size_type s) throw(stream_exception) {
		assert(m_open);
		m_size = s;
		m_fileAccessor->truncate(s);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief file_base destructor.
	///////////////////////////////////////////////////////////////////////////
	~file_base();

	stream_size_type m_size;
	static block_t m_emptyBlock;

	file_base(memory_size_type item_size,
			  double blockFactor=1.0,
			  file_accessor::file_accessor * fileAccessor=NULL);

	void create_block();
	void delete_block();
	block_t * get_block(stream_size_type block);
	void free_block(block_t * block);


	// TODO This should really be a hash map
	boost::intrusive::list<block_t> m_used;
	boost::intrusive::list<block_t> m_free;
};


class file_stream_base: public file_base_base_crtp<file_stream_base> {
public:
	typedef file_base_base_crtp<file_stream_base> p_t;

	inline ~file_stream_base() {
		close();
	}


	/////////////////////////////////////////////////////////////////////////
	/// \brief Close the file and release resources.
	///
	/// This will close the file and resources used by buffers and such.
	/////////////////////////////////////////////////////////////////////////
	inline void close() throw(stream_exception) {
		if (m_open) flush_block();
		tpie_delete_array(m_block.data, m_itemSize * m_blockItems);
		m_block.data = 0;
		p_t::close();
	}

	
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
	/// \copydoc file_base::size()
	/// \sa file_base::size()
	/////////////////////////////////////////////////////////////////////////
	inline stream_size_type size() const throw() {
		// XXX update_vars changes internal state in a way that is not visible
		// through the class interface.
		// therefore, a const_cast is warranted.
		const_cast<file_stream_base*>(this)->update_vars();
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
		const_cast<file_stream_base*>(this)->update_vars();
		if (m_index > 0) return true;
		return m_block.number != 0;
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
					m_index = static_cast<memory_size_type>(new_index);
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

	///////////////////////////////////////////////////////////////////////////
	/// \copydoc file_base::truncate()
	/// \sa file_base::truncate()
	///
	/// Note that when using a file_stream the stream will automatically be
	/// rewound if it is beyond the new end of the file. 
	///////////////////////////////////////////////////////////////////////////
	inline void truncate(stream_size_type size) {
		stream_size_type o=offset();
		flush_block();
		m_block.number = std::numeric_limits<stream_size_type>::max();
		m_nextBlock = std::numeric_limits<stream_size_type>::max();
		m_nextIndex = std::numeric_limits<memory_size_type>::max();
		m_index = std::numeric_limits<memory_size_type>::max();
		m_size = size;
		m_fileAccessor->truncate(size);
		seek(std::min(o, size));
	}


	void swap(file_stream_base & other) {
		using std::swap;
		swap(m_index,           other.m_index);
		swap(m_nextBlock,       other.m_nextBlock);
		swap(m_nextIndex,       other.m_nextIndex);
		swap(m_blockStartIndex, other.m_blockStartIndex);
		swap(m_blockItems,      other.m_blockItems);
		swap(m_blockSize,       other.m_blockSize);
		swap(m_size,            other.m_size);
		swap(m_canRead,         other.m_canRead);
		swap(m_canWrite,        other.m_canWrite);
		swap(m_itemSize,        other.m_itemSize);
		swap(m_open,            other.m_open);
		swap(m_fileAccessor,    other.m_fileAccessor);
		swap(m_block.size,      other.m_block.size);
		swap(m_block.number,    other.m_block.number);
		swap(m_block.dirty,     other.m_block.dirty);
		swap(m_block.data,      other.m_block.data);
		swap(m_ownedTempFile,   other.m_ownedTempFile);
		swap(m_tempFile,        other.m_tempFile);
	}

	inline file_stream_base(double blockFactor, 
							file_accessor::file_accessor * fileAccessor,
							memory_size_type itemSize) {
		m_size = 0;
		m_itemSize = itemSize;
		m_open = false;
		if (fileAccessor == 0)
			fileAccessor = new default_file_accessor();
		m_fileAccessor = fileAccessor;

		m_blockSize = block_size(blockFactor);
		m_blockItems = m_blockSize/m_itemSize;

		m_blockStartIndex = 0;
		m_nextBlock = std::numeric_limits<stream_size_type>::max();
		m_nextIndex = std::numeric_limits<memory_size_type>::max();
		m_index = std::numeric_limits<memory_size_type>::max();
		m_block.data = 0;
		m_tempFile = 0;
	}

	inline void open_inner(const std::string & path,
						   access_type accessType,
						   memory_size_type userDataSize) throw (stream_exception) {
		p_t::open_inner(path, accessType, userDataSize);
		
		m_blockStartIndex = 0;
		m_nextBlock = std::numeric_limits<stream_size_type>::max();
		m_nextIndex = std::numeric_limits<memory_size_type>::max();
		m_index = std::numeric_limits<memory_size_type>::max();

		m_block.size = 0;
		m_block.number = std::numeric_limits<stream_size_type>::max();
		m_block.dirty = false;
		m_block.data = tpie_new_array<char>(m_blockItems * m_itemSize);
		
		initialize();
		seek(0);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Use file_accessor to fetch indicated block number into m_block.
	///////////////////////////////////////////////////////////////////////////
	inline void get_block(stream_size_type block) {
		// If the file contains n full blocks (numbered 0 through n-1), we may
		// request any block in {0, 1, ... n}

		// If the file contains n-1 full blocks and a single non-full block, we may
		// request any block in {0, 1, ... n-1}

		// We capture this restraint with the check:
		if (block * static_cast<stream_size_type>(m_blockItems) > size()) {
			throw end_of_stream_exception();
		}

		m_block.dirty = false;
		m_block.number = block;

		// calculate buffer size
		m_block.size = m_blockItems;
		if (static_cast<stream_size_type>(m_block.size) + m_block.number * static_cast<stream_size_type>(m_blockItems) > size())
			m_block.size = static_cast<memory_size_type>(size() - m_block.number * m_blockItems);

		// populate buffer data
		if (m_block.size > 0 &&
			m_fileAccessor->read_block(m_block.data, m_block.number, m_block.size) != m_block.size) {
			throw io_exception("Incorrect number of items read");
		}
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Fetch block from disk as indicated by m_nextBlock, writing old
	/// block to disk if needed.
	///////////////////////////////////////////////////////////////////////////
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

	///////////////////////////////////////////////////////////////////////////
	/// \brief Write block to disk.
	///////////////////////////////////////////////////////////////////////////
	inline void flush_block() {
		if (m_block.dirty) {
			assert(m_canWrite);
			update_vars();
			m_fileAccessor->write_block(m_block.data, m_block.number, m_block.size);
		}
		m_block.dirty = false;
	}

	inline void update_vars() {
		if (m_block.dirty && m_index != std::numeric_limits<memory_size_type>::max()) {
			assert(m_index <= m_blockItems);
			m_block.size = std::max(m_block.size, m_index);
			m_size = std::max(m_size, static_cast<stream_size_type>(m_index)+m_blockStartIndex);
			if (m_tempFile) 
				m_tempFile->update_recorded_size(m_fileAccessor->byte_size());
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

   	memory_size_type m_index;
	stream_size_type m_nextBlock;
	memory_size_type m_nextIndex;
	stream_size_type m_blockStartIndex;


	struct block_t {
		memory_size_type size;
		stream_size_type number;
		bool dirty;
		char * data;
	};

	block_t m_block;
};
  
} //namespace tpie
#endif //__TPIE_FILE_BASE_H__
