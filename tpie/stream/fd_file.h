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

#ifndef __TPIE_STREAM_FD_FILE_H__
#define __TPIE_STREAM_FD_FILE_H__

#ifdef WIN32

#else
#include <tpie/stream/posix_bte.h>
#endif

#include <tpie/stream/concepts.h>
#include <tpie/stream/exception.h>
#include <boost/cstdint.hpp>
#include <limits>
namespace tpie {
namespace stream {

#ifdef WIN32
#else
typedef posix_block_transfer_engine default_bte;
#endif

template <typename BTE>
class fd_file_base {
public:
	typedef BTE bte_type;
	TPIE_CONCEPT_ASSERT((block_transfer_engine<bte_type>));
	
protected:
	size_type blockItems;
	offset_type m_size;
	bool canRead;
	bool canWrite;

	fd_file_base(size_type blockSize, size_type itemSize, bool canRead, bool canWrite, boost::uint64_t typeMagic);
	
	struct block_t {
		size_type size;
		size_type usage;
		offset_type number;
		bool dirty;
 		block_t * next;
		char data[0];
	};
	
	void create_block();
	void delete_block();
	block_t * get_block(offset_type block);
	void free_block(block_t * block);

	block_t emptyBlock;
private:
	//TODO this should realy be a hash map
	size_type itemSize;
	block_t * firstUsed;
	block_t* firstFree;
	bte_type bte;
public:
	inline void open() {
		bte.open();
		m_size = bte.size();
	}
	inline void open(const std::string & path) {
		bte.open(path);
		m_size = bte.size();
	}
	inline void close() {bte.close();}
	offset_type size() const {return m_size;}

	class stream {
	protected:
		fd_file_base & file;
		size_type index;
		offset_type nextBlock;
		size_type nextIndex;
		block_t * block;
		void update_vars();
		void update_block();
		inline size_type block_items() const {return file.blockItems;}
		inline void write_update() {
			block->dirty = true;
			block->size = std::max(block->size, index);
			file.m_size = std::max(file.m_size, (offset_type)index+block->number*(offset_type)file.blockItems);
		}
	public:
		stream(fd_file_base & file, offset_type offset);
		~stream();
		void seek(offset_type offset);

 		inline offset_type size() const {return file.size();}

 		inline offset_type offset() const {
 			if (nextBlock == std::numeric_limits<offset_type>::max())
 				return index + block->number * file.blockItems;
 			return nextIndex + nextBlock * file.blockItems;
 		}

 		inline bool has_more() const {
 			if (index < block->size) return true;
 			return offset() < file.size();
 		}
	};

	~fd_file_base();
};

template <typename T, bool canRead, bool canWrite, size_type blockSize=1024*1024, typename BTE=default_bte>
class fd_file: public fd_file_base<BTE> {
public:
 	typedef T item_type;

	inline fd_file(boost::uint64_t typeMagic=0):
		fd_file_base<BTE>(blockSize, sizeof(T), canRead, canWrite, typeMagic) {};

 	class stream: public fd_file_base<BTE>::stream {
	public:
		typedef T item_type;
		typedef fd_file file_type;
		
	private:
		typedef typename fd_file_base<BTE>::stream parent_t;
		typedef typename fd_file::block_t block_t;
		using parent_t::block;
		using parent_t::file;
		using parent_t::index;
		using parent_t::block_items;
	public:
		static size_type memory(size_type count=1) {
			return (sizeof(stream) + blockSize + sizeof(block_t))*count;
		}

		stream(file_type & f, offset_type o):
			parent_t(f, o) {}

 		inline const item_type & read() {
			if (index >= block->size) {
				if(parent_t::offset() >= file.size())
					throw end_of_stream_exception("");
				parent_t::update_block();
			}
			return reinterpret_cast<T*>(block->data)[index++];
		}

 		inline void write(const item_type& item) {
			if(index >= block_items()) parent_t::update_block();
			reinterpret_cast<T*>(block->data)[index++] = item;
			if (canRead) parent_t::write_update();
		}
 	};

 	static size_type memory(size_type count=1) {
		return sizeof(fd_file) * count;
 	}
};

}
}

#endif //__TPIE_STREAM_FD_FILE_H
