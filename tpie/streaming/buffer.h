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

#ifndef _TPIE_STREAMING_BUFFER_H
#define _TPIE_STREAMING_BUFFER_H
#include <tpie/streaming/stream.h>
#include <tpie/streaming/memory.h>
#include <utility>

namespace tpie {
namespace streaming {

template <class item_t, class super_t>
class buffer_base: public memory_split {
public:
	typedef item_t item_type;
protected:
	std::vector<item_type> items;
	item_t * buff;
	TPIE_OS_SIZE_T buffSize;
	TPIE_OS_SIZE_T buffIndex;
	ami::stream<item_type> * stream;
	stream_sink< ami::stream<item_type> > * sink;
public:
	TPIE_OS_SIZE_T memoryBase() {
		return sizeof(super_t);
	};

	TPIE_OS_SIZE_T minimumMemoryIn() {
		//TODO substract allocation overhead here
		return memoryBase() + stream_sink<ami::stream<item_type> >::memory(1);
	}
	
	inline void begin(TPIE_OS_OFFSET size=0) {
		TPIE_OS_SIZE_T bs = std::min(memoryIn() - minimumMemoryIn(), 
									 memoryOut() - minimumMemoryOut());
		buffIndex = 0;
		buffSize = std::min(size, TPIE_OS_OFFSET(bs/sizeof(item_t)));
		buff = new item_type[buffSize];
		stream = NULL;
		sink = NULL;
	}
	
	inline void push(const item_type & item) {
		if (stream == NULL) {
			if (buffIndex < buffSize) {
				buff[buffIndex++] = item;
				return;
			}
			stream = new ami::stream<item_type>();
			sink = new stream_sink< ami::stream<item_type> >(stream);
			sink->begin();
		}
		sink->push(item);
	}
	
};
		
template <class T>
class pull_buffer: public buffer_base<T, pull_buffer<T> > {
public:
	typedef T item_type;
private:
	typedef buffer_base<T, pull_buffer<T> > parent_t;
	using parent_t::buff;
	using parent_t::buffIndex;
	using parent_t::sink;
	using parent_t::stream;
	using parent_t::memoryBase;
	pull_stream_source<ami::stream<item_type> > * source;
	TPIE_OS_SIZE_T index;
public:
	
	pull_buffer(): source(NULL), index(0) {};

	void end() {
		if (sink) {
			sink->end();
			delete sink;
			sink = NULL;
		}
	}

	TPIE_OS_SIZE_T minimumMemoryOut() {
		//TODO substract allocation overhead here
		return memoryBase() + pull_stream_source<ami::stream<item_type> >::memory(1);
	}
	
	inline void beginPull() {
		if (stream)	{
			stream->seek(0);
			source = new pull_stream_source<ami::stream<item_type> >(stream);
		}
	}
	

	const item_type & pull() {
		if(index < buffIndex) return buff[index++];
		return source->pull();
	}

	bool atEnd() {
		if (source) return source->atEnd();
		return index == buffIndex;
	}

	void endPull() {
		if (buff) {
			delete[] buff;
			buff=NULL;
		}
		if (source) {
			source->free();
			delete source;
			source = NULL;
		}
		if (stream) {
			delete stream;
			stream = NULL;
		}
	}
};

template <class dest_t> 
class buffer: public buffer_base<typename dest_t::item_type, buffer<dest_t> > {
public:
	typedef typename dest_t::item_type item_type;
private:
	typedef buffer_base<item_type, buffer<dest_t> > parent_t;
	using parent_t::buff;
	using parent_t::buffIndex;
	using parent_t::stream;
	using parent_t::sink;
	using parent_t::memoryBase;
	dest_t & dest;
public:
	buffer(dest_t & d): dest(d) {};

	TPIE_OS_SIZE_T minimumMemoryOut() {
		//TODO substract allocation overhead here
		return memoryBase() + pull_stream_source<ami::stream<item_type> >::memory(1);
	}
	
	void end() {
		if (sink) {
			sink->end();
			delete sink;
		}
		dest.begin(buffIndex + stream?stream->stream_len():0);
		for(size_t i=0; i < buffIndex; ++i)
			dest.push(buff[i]);
		if(stream) {
			stream->seek(0);
			pull_stream_source<ami::stream<item_type> > source(stream);
			while(!source.atEnd())
				dest.push(source.pull());
			delete stream;
			stream = NULL;
		}
		if (buff) {
			delete[] buff;
			buff = NULL;
		}
		dest.end();
	}
	
	virtual void memoryNext(std::vector<memory_base *> & next) {
		next.push_back(&dest);
	}
};

}
}

#endif // _TPIE_STREAMING_BUFFER_H
