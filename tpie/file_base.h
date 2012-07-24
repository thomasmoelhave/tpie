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

///////////////////////////////////////////////////////////////////////////////
/// \file file_base.h  Basic file and stream operations.
///////////////////////////////////////////////////////////////////////////////

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
#include <tpie/cache_hint.h>
#include <tpie/access_type.h>
#include <tpie/types.h>

namespace tpie {

#ifndef WIN32
typedef tpie::file_accessor::stream_accessor<tpie::file_accessor::posix> default_file_accessor;
#else //WIN32
typedef tpie::file_accessor::stream_accessor<tpie::file_accessor::win32> default_file_accessor;
#endif //WIN32

///////////////////////////////////////////////////////////////////////////////
/// \brief Get the TPIE block size.
/// This can be changed by setting the TPIE_BLOCK_SIZE environment variable
/// or by calling the set_block_size method.
///
/// The default is 2 MiB (2**21 bytes).
///////////////////////////////////////////////////////////////////////////////
memory_size_type get_block_size();

///////////////////////////////////////////////////////////////////////////////
/// \brief Set the TPIE block size.
///
/// It is not safe to change the block size once TPIE has been initialized.
///////////////////////////////////////////////////////////////////////////////
void set_block_size(memory_size_type block_size);

///////////////////////////////////////////////////////////////////////////////
/// \brief Base class of classes that access files.
///
/// Inheriting classes may wish to override open_inner() and close(), e.g. to
/// initialize and deinitialize block buffers. open_inner() in the inheriting
/// class will not be called twice in a row without an intervening call to
/// close(). The default implementation of open_inner() passes the open on to
/// the file accessor and sets some attributes.
///////////////////////////////////////////////////////////////////////////////
template <typename child_t>
class file_base_crtp {
	child_t & self() {return *static_cast<child_t *>(this);}
	const child_t & self() const {return *static_cast<const child_t *>(this);}

public:
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
		return static_cast<memory_size_type>(get_block_size() * blockFactor);
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

	///////////////////////////////////////////////////////////////////////////
	/// \brief Amount of memory used by a single block given the block factor.
	///////////////////////////////////////////////////////////////////////////
	static inline memory_size_type block_memory_usage(double blockFactor) {
		return block_size(blockFactor);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the number of items per block.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type block_items() const {
		return m_blockItems;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get the size of a block in bytes.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type block_size() const {
		return m_blockSize;
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Read the user data associated with the file.
	///
	/// \param data Where to store the user data.
	/// \tparam TT The type of user data. sizeof(TT) must be less than or equal
	/// to the maximum user data size of the stream. TT must be trivially
	/// copyable.
	///////////////////////////////////////////////////////////////////////////
	template <typename TT>
	void read_user_data(TT & data) throw(stream_exception) {
		assert(m_open);
		if (sizeof(TT) != user_data_size()) throw io_exception("Wrong user data size");
		m_fileAccessor->read_user_data(reinterpret_cast<void*>(&data), sizeof(TT));
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Read variable length user data associated with the file.
	///
	/// \param data The buffer in which to write data.
	/// \param count The size of the buffer.
	/// \returns Number of bytes of user data actually read.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type read_user_data(void * data, memory_size_type count) {
		assert(m_open);
		return m_fileAccessor->read_user_data(data, count);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Write user data to the stream.
	///
	/// \param data The user data to store in the stream.
	/// \tparam TT The type of user data. sizeof(TT) must be less than or equal
	/// to the maximum user data size of the stream. TT must be trivially
	/// copyable.
	///////////////////////////////////////////////////////////////////////////
	template <typename TT>
	void write_user_data(const TT & data) throw(stream_exception) {
		assert(m_open);
		if (sizeof(TT) > max_user_data_size()) throw io_exception("Wrong user data size");
		m_fileAccessor->write_user_data(reinterpret_cast<const void*>(&data), sizeof(TT));
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Write variable length user data associated with the file.
	///
	/// Throws a stream_exception if the size of the user data exceeds the
	/// maximum user data size of the stream.
	///
	/// \param data The buffer from which to read data.
	/// \param count The size of the user data.
	///////////////////////////////////////////////////////////////////////////
	void write_user_data(const void * data, memory_size_type count) {
		assert(m_open);
		m_fileAccessor->write_user_data(data, count);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get current user data size.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type user_data_size() const {
		assert(m_open);
		return m_fileAccessor->user_data_size();
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Get maximum user data size.
	///////////////////////////////////////////////////////////////////////////
	memory_size_type max_user_data_size() const {
		assert(m_open);
		return m_fileAccessor->max_user_data_size();
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

protected:
	inline void open_inner(const std::string & path,
						   access_type accessType,
						   memory_size_type userDataSize,
						   cache_hint cacheHint) throw(stream_exception) {
		m_canRead = accessType == access_read || accessType == access_read_write;
		m_canWrite = accessType == access_write || accessType == access_read_write;
		m_fileAccessor->open(path, m_canRead, m_canWrite, m_itemSize, m_blockSize, userDataSize, cacheHint);
		m_size = m_fileAccessor->size();
		m_open = true;
	}


	file_base_crtp(memory_size_type itemSize, double blockFactor,
				   file_accessor::file_accessor * fileAccessor);


public:
	/////////////////////////////////////////////////////////////////////////
	/// \brief Open a file.
	///
	/// \param path The path of the file to open.
	/// \param accessType The mode of operation.
	/// \param userDataSize The size of the user data we want to store in the
	/// file.
	/////////////////////////////////////////////////////////////////////////
	inline void open(const std::string & path,
					 access_type accessType=access_read_write,
					 memory_size_type userDataSize=0,
					 cache_hint cacheHint=access_sequential) throw (stream_exception) {
		self().close();
		self().open_inner(path, accessType, userDataSize, cacheHint);
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Open an anonymous temporary file. The temporary file is deleted
	/// when this file is closed.
	/////////////////////////////////////////////////////////////////////////
	inline void open(memory_size_type userDataSize=0,
					 cache_hint cacheHint=access_sequential) throw (stream_exception) {
		self().close();
		m_ownedTempFile.reset(tpie_new<temp_file>());
		m_tempFile=m_ownedTempFile.get();
		self().open_inner(m_tempFile->path(), access_read_write, userDataSize, cacheHint);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief Open a temporary file. The temporary file is not deleted when
	/// this file is closed, so several tpie::file objects may use the same
	/// temporary file consecutively.
	///////////////////////////////////////////////////////////////////////////
	inline void open(temp_file & file,
					 access_type accessType=access_read_write,
					 memory_size_type userDataSize=0,
					 cache_hint cacheHint=access_sequential) throw (stream_exception) {
		self().close();
		m_tempFile=&file;
		self().open_inner(m_tempFile->path(), accessType, userDataSize, cacheHint);
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

	///////////////////////////////////////////////////////////////////////////
	/// \brief Check if file is open.
	///////////////////////////////////////////////////////////////////////////
	inline bool is_open() const {
		return m_open;
	}

protected:
	template <typename BT>
	void read_block(BT & b, stream_size_type block);
	void get_block_check(stream_size_type block);

public:
	/////////////////////////////////////////////////////////////////////////
	/// \brief Get the size of the file measured in items.
	/// If there are streams of this file that have extended the stream length
	/// but have not yet flushed these writes, we might report an incorrect
	/// size.
	///
	/// \returns The number of items in the file.
	/////////////////////////////////////////////////////////////////////////
	inline stream_size_type file_size() const throw() {
		return m_size;
	}

protected:
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
class stream_crtp {
public:
  	/** Type describing how we should interpret the offset supplied to seek. */
	enum offset_type {
		beginning,
		end,
		current
	};

private:
	inline child_t & self() {return *static_cast<child_t*>(this);}
	inline const child_t & self() const {return *static_cast<const child_t*>(this);}

public:
	/////////////////////////////////////////////////////////////////////////
	/// \brief Moves the logical offset in the stream.
	///
	/// \param offset Where to move the logical offset to.
	/// \param whence Move the offset relative to what.
	/////////////////////////////////////////////////////////////////////////
	inline void seek(stream_offset_type offset, offset_type whence=beginning) throw(stream_exception) {
		assert(self().__file().is_open());
		if (whence == end)
			offset += self().size();
		else if (whence == current) {
			// are we seeking into the current block?
			if (offset >= 0 || static_cast<stream_size_type>(-offset) <= m_index) {
				stream_size_type new_index = static_cast<stream_offset_type>(offset+m_index);
				
				if (new_index < self().__file().block_items()) {
					self().update_vars();
					m_index = static_cast<memory_size_type>(new_index);
					return;
				}
			}
			
			offset += self().offset();
		}
		if (0 > offset || (stream_size_type)offset > self().size())
			throw io_exception("Tried to seek out of file");
		self().update_vars();
		stream_size_type b = static_cast<stream_size_type>(offset) / self().__file().block_items();
		m_index = static_cast<memory_size_type>(offset - b* self().__file().block_items());
		if (b == self().__block().number) {
			m_nextBlock = std::numeric_limits<stream_size_type>::max();
			m_nextIndex = std::numeric_limits<memory_size_type>::max();
			assert(self().offset() == (stream_size_type)offset);
			return;
		}
		m_nextBlock = b;
		m_nextIndex = m_index;
		m_index = std::numeric_limits<memory_size_type>::max();
		assert(self().offset() == (stream_size_type)offset);
	}

protected:
	inline void initialize() {
		m_nextBlock = std::numeric_limits<stream_size_type>::max();
		m_nextIndex = std::numeric_limits<memory_size_type>::max();
		m_index = std::numeric_limits<memory_size_type>::max();
	}

public:
	/////////////////////////////////////////////////////////////////////////
	/// \brief Calculate the current offset in the stream.
	///
	/// \returns The current offset in the stream
	/////////////////////////////////////////////////////////////////////////
	inline stream_size_type offset() const throw() {
		assert(self().__file().is_open());
		if (m_nextBlock == std::numeric_limits<stream_size_type>::max())
			return m_index + m_blockStartIndex;
		return m_nextIndex + m_nextBlock * self().__file().block_items();
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
		assert(self().__file().is_open());
		if (m_index < self().__block().size ) return true;
		return offset() < self().size();
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Check if we can read an item with read_back().
	///
	/// \returns Whether or not we can read an item with read_back().
	/////////////////////////////////////////////////////////////////////////
	inline bool can_read_back() const throw() {
		assert(self().__file().is_open());
		if (m_nextBlock == std::numeric_limits<stream_size_type>::max())
			return m_index > 0 || m_blockStartIndex > 0;
		else
			return m_nextIndex > 0 || m_nextBlock > 0;
	}

	/////////////////////////////////////////////////////////////////////////
	/// \brief Get the size of the file measured in items.
	///
	/// \returns The number of items in the file.
	/////////////////////////////////////////////////////////////////////////
	inline stream_size_type size() const throw() {
		// XXX update_vars changes internal state in a way that is not visible
		// through the class interface.
		// therefore, a const_cast is warranted.
		const_cast<child_t&>(self()).update_vars();
		return self().__file().file_size();
	}

protected:
	///////////////////////////////////////////////////////////////////////
	/// \brief Fetch block from disk as indicated by m_nextBlock, writing old
	/// block to disk if needed.
	/// Update m_block, m_index, m_nextBlock and m_nextIndex. If
	/// m_nextBlock is maxint, use next block is the one numbered
	/// m_block->number+1. m_index is updated with the value of
	/// m_nextIndex.
	///////////////////////////////////////////////////////////////////////
	void update_block();

protected:
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
};


class file_base: public file_base_crtp<file_base> {
	typedef file_base_crtp<file_base> p_t;
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

	inline void update_size(stream_size_type size) {
		m_size = std::max(m_size, size);
		if (m_tempFile) 
			m_tempFile->update_recorded_size(m_fileAccessor->byte_size());
	}

public:
	inline stream_size_type size() const throw() {
		return file_size();
	}

	void close();

	///////////////////////////////////////////////////////////////////////////
	/// \brief Stream in file. We support multiple streams per file.
	///////////////////////////////////////////////////////////////////////////
	class stream: public stream_crtp<stream> {
		typedef stream_crtp<stream> p_t;

		friend class stream_crtp<stream>;
		friend struct stream_item_array_operations;

		block_t & __block() {return *m_block;}
		const block_t & __block() const {return *m_block;}
		inline file_base & __file() {return m_file;}
		inline const file_base & __file() const {return m_file;}

	protected:
		void update_block_core();

		inline void update_vars() {}
		/** Associated file object. */
		file_base & m_file;

		/** Current block. May be equal to &m_file.m_emptyBlock to indicate no
		 * current block. */
		block_t * m_block;

	public:
		///////////////////////////////////////////////////////////////////////
		/// Fetch number of items per block.
		///////////////////////////////////////////////////////////////////////
		inline memory_size_type block_items() const {return m_file.m_blockItems;}

	protected:
		///////////////////////////////////////////////////////////////////////
		/// Call whenever the current block buffer is modified. Since we
		/// support multiple streams per block, we must always keep
		/// m_block->size updated when m_block is the trailing block (or the
		/// only block) in the file. For the same reasons we keep m_file.m_size
		/// updated.
		///////////////////////////////////////////////////////////////////////
		inline void write_update() {
			m_block->dirty = true;
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


	protected:
		///////////////////////////////////////////////////////////////////////
		/// \brief Set up block buffers and offsets.
		///////////////////////////////////////////////////////////////////////
		inline void initialize() {
			if (m_block != &m_file.m_emptyBlock) m_file.free_block(m_block);
			p_t::initialize();
			m_block = &m_file.m_emptyBlock;
		}
	};

	///////////////////////////////////////////////////////////////////////////
	/// \brief Truncate file to given size. May only be used when no streams
	/// are opened to this file.
	///////////////////////////////////////////////////////////////////////////
	void truncate(stream_size_type s) throw(stream_exception) {
		assert(m_open);
		if (!m_used.empty()) {
			throw io_exception("Tried to truncate a file with one or more open streams");
		}
		m_size = s;
		m_fileAccessor->truncate(s);
	}

	///////////////////////////////////////////////////////////////////////////
	/// \brief file_base destructor.
	///////////////////////////////////////////////////////////////////////////
	~file_base();


protected:
	file_base(memory_size_type item_size,
			  double blockFactor=1.0,
			  file_accessor::file_accessor * fileAccessor=NULL);

	void create_block();
	void delete_block();
	block_t * get_block(stream_size_type block);
	void free_block(block_t * block);


	static block_t m_emptyBlock;
	// TODO This should really be a hash map
	boost::intrusive::list<block_t> m_used;
	boost::intrusive::list<block_t> m_free;
};




class file_stream_base: public file_base_crtp<file_stream_base>, public stream_crtp<file_stream_base> {
public:
	typedef file_base_crtp<file_stream_base> p_t;
	typedef stream_crtp<file_stream_base> s_t;

	friend class file_base_crtp<file_stream_base>;

	struct block_t {
		memory_size_type size;
		stream_size_type number;
		bool dirty;
		char * data;
	};

private:
	friend class stream_crtp<file_stream_base>;
	friend struct stream_item_array_operations;
	file_stream_base & __file() {return *this;}
	const file_stream_base & __file() const {return *this;}
	block_t & __block() {return m_block;}
	const block_t & __block() const {return m_block;}
	void update_block_core();

protected:
	file_stream_base(memory_size_type itemSize,
					 double blockFactor,
					 file_accessor::file_accessor * fileAccessor);

	inline ~file_stream_base() {
		close();
	}


public:
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

protected:
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

	inline void open_inner(const std::string & path,
						   access_type accessType,
						   memory_size_type userDataSize,
						   cache_hint cacheHint) throw (stream_exception) {
		p_t::open_inner(path, accessType, userDataSize, cacheHint);
		
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
	void get_block(stream_size_type block);

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
		s_t::initialize();
	}

	inline void write_update() {
		m_block.dirty = true;
	}


	block_t m_block;
};

struct stream_item_array_operations {
	///////////////////////////////////////////////////////////////////////////
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
	///////////////////////////////////////////////////////////////////////////
	template <typename T, typename IT, typename Stream>
	static inline void read(Stream & stream, const IT & start, const IT & end) throw(stream_exception) {
		IT i = start;
		while (i != end) {
			if (stream.m_index >= stream.block_items()) {
				// check to make sure we have enough items in the stream
				stream_size_type offs = stream.offset();
				if (offs >= stream.size()
					|| offs + (end-i) > stream.size()) {

					throw end_of_stream_exception();
				}

				// fetch next block from disk
				stream.update_block();
			}

			T * src = reinterpret_cast<T*>(stream.__block().data) + stream.m_index;

			// either read the rest of the block or until `end'
			memory_size_type count = std::min(stream.block_items()-stream.m_index, static_cast<memory_size_type>(end-i));

			std::copy(src, src + count, i);

			// advance output iterator
			i += count;

			// advance input position
			stream.m_index += count;
		}
	}

	/////////////////////////////////////////////////////////////////////////////
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
	/////////////////////////////////////////////////////////////////////////////
	template <typename T, typename IT, typename Stream>
	static inline void write(Stream & stream, const IT & start, const IT & end) throw(stream_exception) {
		IT i = start;
		while (i != end) {
			if (stream.m_index >= stream.block_items()) stream.update_block();

			size_t streamRemaining = end - i;
			size_t blockRemaining = stream.block_items()-stream.m_index;

			IT till = (blockRemaining < streamRemaining) ? (i + blockRemaining) : end;

			T * dest = reinterpret_cast<T*>(stream.__block().data) + stream.m_index;

			std::copy(i, till, dest);

			stream.m_index += till - i;
			stream.write_update();
			i = till;
		}
	}
};

} //namespace tpie
#endif //__TPIE_FILE_BASE_H__
