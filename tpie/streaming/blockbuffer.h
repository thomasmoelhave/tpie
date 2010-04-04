// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2010, The TPIE development team
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
#ifndef _TPIE_STREAMING_BLOCKBUFFER_H
#define _TPIE_STREAMING_BLOCKBUFFER_H
#include <tpie/file.h>
#include <tpie/streaming/util.h>
namespace tpie {
namespace streaming {

template <typename dest_t>
class push_block_buffer: public push_single<push_block_buffer<dest_t>, dest_t> {
private:
	typedef push_single<push_block_buffer<dest_t>, dest_t> parent_t;
public:
	typedef typename dest_t::item_type item_type;
	typedef typename dest_t::begin_data_type begin_data_type;
	typedef typename dest_t::end_data_type end_data_type;

	push_block_buffer(dest_t & dest, double blockFactor=1.0):
		parent_t(dest, 0.0),
		bufferSize(file_base::block_size(blockFactor)/sizeof(item_type)) {}

	void begin(stream_size_type items=max_items, begin_data_type * data=0) {
		dest().begin(items, data);
		bufferCount = 0;
		buffer = new item_type[bufferSize];
	}

   	inline void push(const item_type & item) {
		buffer[bufferCount++] = item;
		if (bufferCount == bufferSize)
			flush();
	}

	void end(end_data_type * data=0) {
		flush();
		delete[] buffer;
		dest().end(data);
	}

	memory_size_type minimum_memory() {
		return parent_t::base_memory() + //Space for our self
			1*MM_manager.space_overhead() +
			bufferSize*sizeof(item_type);
	}
private:
	item_type * buffer;
	memory_size_type bufferCount;
	memory_size_type bufferSize;
	//Explecitly not inlined
	void flush() {
		for (memory_size_type i=0; i < bufferCount; ++i)
			dest().push(buffer[i]);
		bufferCount = 0;
	}

	using parent_t::dest;
};

template <typename source_t>
class pull_block_buffer: public pull_single< pull_block_buffer<source_t>, source_t > {
private:
	typedef pull_single< pull_block_buffer<source_t>, source_t > parent_t;
public:
	typedef typename source_t::pull_begin_data_type pull_begin_data_type;
	typedef typename source_t::pull_end_data_type pull_end_data_type;
	typedef typename source_t::pull_type pull_type;

	pull_block_buffer(source_t & source, double blockFactor=1.0):
		parent_t(source, 0.0),
		bufferSize( file_base::block_size(blockFactor) / sizeof(pull_type) ) {};

	void pull_begin(stream_size_type * size=0, pull_begin_data_type * data=0) {
		source().pull_begin(size, data);
		bufferSize = 1024;
		buffer = new pull_type[bufferSize];
		fill();
	}

	void pull_end(pull_end_data_type * data=0) {
		delete[] buffer;
		source().pull_end(data);
	}

	inline bool can_pull() const {
		return m_canPull || (index < bufferCount);
	}

	inline pull_type & pull() {
		if (index == bufferCount) fill();
		return buffer[index++];
	}

	memory_size_type minimum_memory() {
		return parent_t::base_memory() + //Space for our self
			1*MM_manager.space_overhead() +
			bufferSize*sizeof(pull_type);
	}
private:
	bool m_canPull;
	memory_size_type index;
	memory_size_type bufferCount;
	memory_size_type bufferSize;
	pull_type * buffer;
	using parent_t::source;

	void fill() {
		index=0;
		for (bufferCount=0; bufferCount < bufferSize && source().can_pull(); ++bufferCount)
			buffer[bufferCount] = source().pull();
		m_canPull = source().can_pull();
	}
};


}
}

#endif //_TPIE_STREAMING_BLOCKBUFFER_H
