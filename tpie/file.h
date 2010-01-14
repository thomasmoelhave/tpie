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

#ifndef __TPIE_FILE_H__
#define __TPIE_FILE_H__

//#include <tpie/stream/stdio_bte.h>
#include <tpie/mm_base.h>
#include <tpie/mm_manager.h>
///#include <tpie/stream/concepts.h>
#include <tpie/exception.h>
#include <boost/cstdint.hpp>
#include <limits>

#include <tpie/file_accessor/stdio.h>

namespace tpie {

typedef tpie::file_accessor::stdio default_file_accessor;

class file_base {
public:
	enum access_type {
		read, 
		write, 
		read_write
	};

	bool m_canWrite;
protected:
	memory_size_type m_blockItems;
	stream_size_type m_size;
	bool m_canRead;

	static inline memory_size_type blockSize(float blockFactor) {
		return 2 * 1024*1024 * blockFactor;
	}

	file_base(memory_size_type item_size, 
			  float blockFactor=1.0, 
			  file_accessor::file_accessor * fileAccessor=NULL);
				  
	struct block_t {
		memory_size_type size;
		memory_size_type usage;
		stream_size_type number;
		bool dirty;
 		block_t * next;
		char data[0];
	};
	
	void create_block();
	void delete_block();
	block_t * get_block(stream_size_type block);
	void free_block(block_t * block);

	block_t m_emptyBlock;
private:
	//TODO this should realy be a hash map
	memory_size_type m_itemSize;
	block_t * m_firstUsed;
	block_t* m_firstFree;
	file_accessor::file_accessor * m_fileAccessor;
public:
	template <typename T>
	void read_user_data(T & ud) {
		if (sizeof(T) != m_fileAccessor->user_data_size()) throw io_exception("Wrong user data size");
		m_fileAccessor->read_user_data(reinterpret_cast<void*>(&ud));
	}

	template <typename T>
	void write_user_data(const T & ud) {
		if (sizeof(T) != m_fileAccessor->user_data_size()) throw io_exception("Wrong user data size");
		m_fileAccessor->write_user_data(reinterpret_cast<const void*>(&ud));
	}
	
	inline void close() {
		m_fileAccessor->close();
	}

	inline void open(const std::string & path, access_type accessType, memory_size_type userDataSize=0) {
		close();
		m_canRead = accessType == read || accessType == read_write;
		m_canWrite = accessType == write || accessType == read_write,
		m_fileAccessor->open(path, m_canRead, m_canWrite, m_itemSize, userDataSize);
		m_size = m_fileAccessor->size();
	}

	inline stream_size_type size() const {return m_size;}

	inline const std::string & path() const {return m_fileAccessor->path();}

	class stream {
	protected:
		file_base & m_file;
		memory_size_type m_index;
		stream_size_type m_nextBlock;
		memory_size_type m_nextIndex;
		block_t * m_block;
		inline void update_vars() {
			if (m_index != std::numeric_limits<memory_size_type>::max()) 
				m_block->size = std::max(m_block->size, m_index);
			if (m_index != std::numeric_limits<memory_size_type>::max() &&
				m_block->number != std::numeric_limits<stream_size_type>::max())
				m_file.m_size = std::max(m_file.m_size, m_block->number * static_cast<stream_size_type>(m_file.m_blockItems) + static_cast<stream_size_type>(m_index));
		}

		void update_block();
		inline memory_size_type block_items() const {return m_file.m_blockItems;}
		inline void write_update() {
			m_block->dirty = true;
			m_block->size = std::max(m_block->size, m_index);
			m_file.m_size = std::max(m_file.m_size, static_cast<stream_size_type>(m_index)+m_block->number*static_cast<stream_size_type>(m_file.m_blockItems));
		}
	public:

		stream(file_base & file, stream_size_type offset=0);
		void free();
		inline ~stream() {free();}

		inline void seek(stream_size_type offset) {
			if (offset > size()) 
				throw io_exception("Tried to seek out of file");
			update_vars();
			stream_size_type b = offset / m_file.m_blockItems;
			m_index = offset - b*m_file.m_blockItems;
			if (b == m_block->number) {
				m_nextBlock = std::numeric_limits<stream_size_type>::max();
				m_nextIndex = std::numeric_limits<memory_size_type>::max();
				return;
			}
			m_nextBlock = b;
			m_nextIndex = m_index;
			m_index = std::numeric_limits<memory_size_type>::max();
		}

 		inline stream_size_type size() const {return m_file.size();}

 		inline stream_size_type offset() const {
 			if (m_nextBlock == std::numeric_limits<stream_size_type>::max())
 				return m_index + m_block->number * m_file.m_blockItems;
 			return m_nextIndex + m_nextBlock * m_file.m_blockItems;
 		}

 		inline bool has_more() const {
 			if (m_index < m_block->size) return true;
 			return offset() < size();
 		}
	};

	void truncate(stream_size_type s) {
		m_size = s;
		m_fileAccessor->truncate(s);
	}

	~file_base();
};

template <typename T>
class file: public file_base {
public:
 	typedef T item_type;

	static inline memory_size_type memory_usage(memory_size_type count=1, bool includeDefaultFileAccessor=true) {
		memory_size_type x = sizeof(file) * count;
		if (includeDefaultFileAccessor)
			x += MM_manager.space_overhead()*count + default_file_accessor::memory_usage(count);
		return x;
	}

	file(float blockFactor=1.0, 
		 file_accessor::file_accessor * fileAccessor=NULL):
		file_base(sizeof(T), blockFactor, fileAccessor) {};

 	class stream: public file_base::stream {
	public:
		typedef T item_type;
		typedef file file_type;
	private:
		typedef typename file::block_t block_t;
	public:
		inline static memory_size_type memory_usage(memory_size_type count=1, float blockFactor=1.0) {
			return (sizeof(stream) + blockSize(blockFactor) +  MM_manager.space_overhead() + sizeof(block_t)) * count;
		}

		stream(file_type & file, stream_size_type offset=0):
			file_base::stream(file, offset) {}

 		inline item_type & read() {
			if (m_index >= m_block->size) {
				if (offset() >= m_file.size())
					throw end_of_stream_exception();
				update_block();
			}
			return reinterpret_cast<T*>(m_block->data)[m_index++];
		}

 		inline void write(const item_type& item) {
#ifndef NDEBUG
			if (!m_file.m_canWrite) 
				throw io_exception("Cannot write to read only stream");
#endif
			if (m_index >= block_items()) update_block();
			reinterpret_cast<T*>(m_block->data)[m_index++] = item;
			write_update();
		}

		template <typename IT>
		inline void write(const IT & start, const IT & end) {
			for(IT i=start; i != end; ++i) 
				write(*i);
		};

		template <typename IT>
		inline void read(const IT & start, const IT & end) {
			for(IT i=start; i != end; ++i) 
				*i = read();
		};

 	};

};

}

#endif //__TPIE_STREAM_FD_FILE_H
