// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, 2010, 2012, The TPIE development team
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
#ifndef _TPIE_FILE_H
#define _TPIE_FILE_H

///////////////////////////////////////////////////////////////////////////////
/// \file file.h Streams that support substreams.
///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
//Yes we know you do not support throw(stream_exception)
#pragma warning( disable: 4290 ) 
#endif //_MSC_VER

#include <limits>
#include <tpie/exception.h>
#include <tpie/file_accessor/file_accessor.h>
#ifndef WIN32
#include <tpie/file_accessor/posix.h>
#else ////WIN32
#include <tpie/file_accessor/win32.h>
#endif //WIN32
#include <boost/intrusive/list.hpp>
#include <tpie/tempname.h>

namespace tpie {

#ifndef WIN32
typedef tpie::file_accessor::stream_accessor<tpie::file_accessor::posix> default_file_accessor;
#else //WIN32
typedef tpie::file_accessor::stream_accessor<tpie::file_accessor::win32> default_file_accessor;
#endif //WIN32

#ifdef _MSC_VER
#pragma warning( disable: 4200 )
#endif //_MSC_VER

///////////////////////////////////////////////////////////////////////////////
/// \brief Base class of \ref file.
///////////////////////////////////////////////////////////////////////////////
class file_base {
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
	/// \brief Close the file.
	///
	/// Note all streams into the file must be freed before you call close.
	/////////////////////////////////////////////////////////////////////////
	inline void close() throw(stream_exception) {
		if (m_open) m_fileAccessor->close();
		m_open = false;
		m_tempFile = NULL;
	}

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
					 memory_size_type userDataSize=0) throw(stream_exception) {
		close();
		open_inner(path, accessType, userDataSize);
	}

	inline void open(temp_file & file,
					 access_type accessType=read_write,
					 memory_size_type userDataSize=0) throw(stream_exception) {
		close();
		m_tempFile = &file;
		open_inner(file.path(), accessType, userDataSize);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Calculate the size of the file measured in items.
	///
	/// \returns The number of items is the file.
	/////////////////////////////////////////////////////////////////////////
	inline stream_size_type size() const throw() {
		return m_size;
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

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the number of items per block.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type blockItems() {
		return m_blockItems;
	}

	memory_size_type blockSize() {
		return m_blockSize;
	}

	bool m_open;
protected:
	memory_size_type m_blockItems;
	memory_size_type m_blockSize;
	stream_size_type m_size;
	bool m_canRead;
	bool m_canWrite;
	static block_t m_emptyBlock;

	file_base(memory_size_type item_size,
			  double blockFactor=1.0,
			  file_accessor::file_accessor * fileAccessor=NULL);

	void create_block();
	void delete_block();
	block_t * get_block(stream_size_type block);
	void free_block(block_t * block);

	memory_size_type m_itemSize;
private:
	// TODO This should really be a hash map
	boost::intrusive::list<block_t> m_used;
	boost::intrusive::list<block_t> m_free;
	file_accessor::file_accessor * m_fileAccessor;

	temp_file * m_tempFile;

	inline void open_inner(const std::string & path,
						   access_type accessType=read_write,
						   memory_size_type userDataSize=0) throw(stream_exception) {
		m_canRead = accessType == read || accessType == read_write;
		m_canWrite = accessType == write || accessType == read_write;
		m_fileAccessor->open(path, m_canRead, m_canWrite, m_itemSize, m_blockSize, userDataSize);
		m_size = m_fileAccessor->size();
		m_open = true;
	}
};

///////////////////////////////////////////////////////////////////////////////
/// \brief Central file abstraction.
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class file: public file_base {
public:
	/** Type of items stored in the file. */
 	typedef T item_type;

	///////////////////////////////////////////////////////////////////////////
	/// \brief Calculate the memory usage of a file.
	///////////////////////////////////////////////////////////////////////////
	static inline memory_size_type memory_usage(bool includeDefaultFileAccessor=true) {
		memory_size_type x = sizeof(file);
		if (includeDefaultFileAccessor)
			x += default_file_accessor::memory_usage();
		return x;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Construct a file object with the given block factor and file
	/// accessor.
	/// \param blockFactor The relative size of a block compared to the 
	/// default. To find the block factor corresponding to an absolute
	/// block size, use file_base::calculate_block_factor.
	/// \param fileAccessor The file accessor to use, if none is supplied a
	/// default will be used.
	///////////////////////////////////////////////////////////////////////////
	file(double blockFactor=1.0,
		 file_accessor::file_accessor * fileAccessor=NULL):
		file_base(sizeof(T), blockFactor, fileAccessor) {};

	///////////////////////////////////////////////////////////////////////////
	/// \brief Central stream abstraction. Conceptually compatible with
	/// \ref file_stream.
	///////////////////////////////////////////////////////////////////////////
 	class stream: public file_base::stream {
	public:
		/** Type of items stored in the stream. */
		typedef T item_type;
		/** Type of underlying file object. */
		typedef file file_type;
	private:
		/** Type of block. */
		typedef typename file::block_t block_t;
	public:
		///////////////////////////////////////////////////////////////////////
		/// \brief Calculate the memory usage of a stream.
		///////////////////////////////////////////////////////////////////////
		inline static memory_size_type memory_usage(double blockFactor=1.0) {
			return sizeof(stream) + block_size(blockFactor) +  sizeof(block_t);
		}

		stream(file_type & file, stream_size_type offset=0):
			file_base::stream(file, offset) {}


		///////////////////////////////////////////////////////////////////////
		/// \brief Read a mutable item from the stream.
		///
		/// Don't use this method. Instead, use \ref file<T>::stream::read().
		///
		/// \copydetails file<T>::stream::read()
		///////////////////////////////////////////////////////////////////////
 		inline item_type & read_mutable() {
			assert(m_file.m_open);
			if (m_index >= m_block->size) {
				update_block();
				if (offset() >= m_file.size()) {
					throw end_of_stream_exception();
				}
			}
			return reinterpret_cast<T*>(m_block->data)[m_index++];
		}

		///////////////////////////////////////////////////////////////////////
		/// \brief Read an item from the stream.
		///
		/// Read current item from the stream, and increment the offset by one
		/// item.
		///
		/// This will throw an end_of_stream_exception if there are no more
		/// items left in the stream.
		///
		/// To ensure that no exception is thrown, check that can_read()
		/// returns true.
		///
		/// \returns The item read from the stream.
		///////////////////////////////////////////////////////////////////////
 		inline const item_type & read() {
			return read_mutable();
		}

		///////////////////////////////////////////////////////////////////////
		/// \brief Read an item from the stream.
		///
		/// Decrement the offset by one, and read current item from the stream.
		///
		/// This will throw an end_of_stream_exception if there are no more
		/// items left in the stream.
		///
		/// To ensure that no exception is thrown, check that can_read_back()
		/// returns true.
		///
		/// \returns The item read from the stream.
		///////////////////////////////////////////////////////////////////////
		inline const item_type & read_back() {
			assert(m_file.m_open);
			seek(-1, current);
			const item_type & i = read();
			seek(-1, current);
			return i;
		}

		/////////////////////////////////////////////////////////////////////////
		/// \brief Write an item to the stream.
		///
		/// \param item The item to write to the stream.
		/////////////////////////////////////////////////////////////////////////
 		inline void write(const item_type& item) throw(stream_exception) {
			assert(m_file.m_open);
#ifndef NDEBUG
			if (!m_file.is_writable())
				throw io_exception("Cannot write to read only stream");
#endif
			if (m_index >= block_items()) update_block();
			reinterpret_cast<T*>(m_block->data)[m_index++] = item;
			write_update();
		}

		/////////////////////////////////////////////////////////////////////////
		/// \brief Write several items to the stream.
		///
		/// Implementation note: If your iterator type is efficiently copyable
		/// with std::copy, then this will also write efficiently into the
		/// internal TPIE buffer.
		///
		/// \tparam IT The type of Random Access Iterators used to supply the
		/// items.
		/// \param start Iterator to the first item to write.
		/// \param end Iterator past the last item to write.
		/////////////////////////////////////////////////////////////////////////
		template <typename IT>
		inline void write(const IT & start, const IT & end) {
			assert(m_file.m_open);
			IT i = start;
			while (i != end) {
				if (m_index >= block_items()) update_block();

				IT blockmax = i + (block_items()-m_index);

				T * dest = reinterpret_cast<T*>(m_block->data) + m_index;

				IT till = std::min(end, blockmax);

				std::copy(i, till, dest);

				m_index += till - i;
				write_update();
				i = till;
			}
		}

		///////////////////////////////////////////////////////////////////////
		/// \brief Reads several items from the stream.
		///
		/// Implementation note: If your iterator type is efficiently copyable
		/// with std::copy, then this will also read efficiently from the
		/// internal TPIE buffer.
		///
		/// \tparam IT The type of Random Access Iterators used to supply the
		/// items.
		/// \param start Iterator to the first spot to write to.
		/// \param end Iterator past the last spot to write to.
		///
		/// \throws end_of_stream_exception If there are not enough elements in
		/// the stream to fill all the spots between start and end.
		///////////////////////////////////////////////////////////////////////
		template <typename IT>
		inline void read(const IT & start, const IT & end) {
			assert(m_file.m_open);
			IT i = start;
			while (i != end) {
				if (m_index >= block_items()) {
					// check to make sure we have enough items in the stream
					stream_size_type offs = offset();
					if (offs >= m_file.size()
						|| offs + (end-i) >= m_file.size()) {

						throw end_of_stream_exception();
					}

					// fetch next block from disk
					update_block();
				}

				T * src = reinterpret_cast<T*>(m_block->data) + m_index;

				// either read the rest of the block or until `end'
				memory_size_type count = std::min(block_items()-m_index, static_cast<memory_size_type>(end-i));

				std::copy(src, src + count, i);

				// advance output iterator
				i += count;

				// advance input position
				m_index += count;
			}
		}
 	};
};
}
#endif //_TPIE_FILE_H
