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
#include <tpie/streaming/concepts.h>

namespace tpie {
namespace streaming {

template <typename item_t, 
		  typename begin_data_t=empty_type>
class buffer_base: public memory_split {
public:
	typedef item_t item_type;
protected:
	item_t * buff;
	memory_size_type buffSize;
	memory_size_type buffIndex;

	file_stream<item_type> * stream;
	stream_sink<item_type> * sink;
	
	double blockFactor;
	begin_data_t * beginData;
public:
	buffer_base(): buff(0), stream(0), sink(0), blockFactor(0.0), beginData(0) {}

	memory_size_type minimum_memory_in() {
		//TODO substract allocation overhead here
		return base_memory() 
			+ file_stream<item_type>::memory_usage(1, blockFactor)
			+ stream_sink<item_type>::memory( );
	}
	
	inline void begin(stream_size_type items=max_items, begin_data_t * data=0) {
		memory_size_type bs = std::min(memory_in() - minimum_memory_in(), 
									 memory_out() - minimum_memory_out());
		buffIndex = 0;
		buffSize = std::min(items, stream_size_type(bs/sizeof(item_t)));;
		buff = new item_type[buffSize];
		stream = 0;
		sink = 0;
		beginData = data;
	}
	
	inline void push(const item_type & item) {
		if (stream == 0) {
			if (buffIndex < buffSize) {
				buff[buffIndex++] = item;
				return;
			}
			stream = new file_stream<item_type>(blockFactor);
			sink = new stream_sink<item_type>(*stream);
			sink->begin();
		}
		sink->push(item);
	}
};

 		
template <typename T, 
		  bool backwards=false,
		  typename begin_data_t=empty_type, 
		  typename end_data_type=empty_type,
		  typename pull_begin_data_t=empty_type,
		  typename pull_end_data_t=empty_type>
class pull_buffer: public buffer_base<T, begin_data_t> {
public:
	typedef T item_type;
	typedef T pull_type;
private:
	typedef buffer_base<T, begin_data_t> parent_t;
	using parent_t::buff;
	using parent_t::buffIndex;
	using parent_t::sink;
	using parent_t::stream;
	using parent_t::base_memory;
	using parent_t::blockFactor;
	typedef pull_stream_source<item_type, backwards> source_t;

	pull_stream_source<item_type> * source;
	memory_size_type index;
	end_data_type * end_data;
public:
	memory_size_type base_memory() {
		return sizeof(*this);
	}
	
	pull_buffer(): source(0), index(0) {};

	void end(end_data_type * data=0) {
		end_data=data;
		if (sink) {
			sink->end();
			delete sink;
			sink = 0;
		}
	}

	memory_size_type minimum_memory_out() {
		return base_memory() 
			+ pull_stream_source<item_type>::memory( )
			+ file_stream<item_type>::memory_usage(blockFactor, 1);
	}
	

	inline void pull_begin(stream_size_type * size=0, pull_begin_data_t * data=0) {
		unused(data);
		if (size) *size = stream_size_type(buffIndex) + (stream?stream->size():stream_size_type(0));
		if (stream)
			source = new source_t(*stream);
		if (backwards)
			index=buffIndex-1;
		else
			index=0;
	}
	
	const item_type & pull() {
		if (backwards) {
			if (index != memory_size_type(-1)) return buff[index--];
		} else {
			if (index < buffIndex) return buff[index++];
		}
		return source->pull();
	}

	bool can_pull() {
		if (source) return source->can_pull();
		if (backwards) 
			return index != memory_size_type(-1);
		else
			return index != buffIndex;
	}

	void pull_end(pull_end_data_t * data=0) {
		unused(data);
		if (buff) {
			delete[] buff;
			buff=0;
		}
		if (source) {
			delete source;
			source = 0;
		}
		if (stream) {
			delete stream;
			stream = 0;
		}
	}
};

template <class dest_t, bool backwards=false> 
class buffer: public buffer_base<typename dest_t::item_type, typename dest_t::begin_data_type> {
public:
	typedef typename dest_t::item_type item_type;
	typedef typename dest_t::begin_data_type begin_data_type;
	typedef typename dest_t::end_data_type end_data_type;
	
private:
	typedef buffer_base<item_type, begin_data_type> parent_t;
	typedef pull_stream_source<item_type, backwards> source_t;
	//BOOST_CONCEPT_ASSERT((tpie::streaming::concepts::pushable<dest_t>));

	using parent_t::buff;
	using parent_t::buffIndex;
	using parent_t::stream;
	using parent_t::sink;
	using parent_t::base_memory;
	using parent_t::blockFactor;
	using parent_t::beginData;
	dest_t & dest;
public:
	memory_size_type base_memory() {
		return sizeof(*this);
	}

	buffer(dest_t & d): dest(d) {};

	memory_size_type minimum_memory_out() {
		//TODO substract allocation overhead here
		return base_memory() + file_stream<item_type>::memory_usage(1, blockFactor) 
			+ source_t::memory( );
	}

	void end(end_data_type * endData=0) {
		if (sink) {
			sink->end();
			delete sink;
		}
		std::cout << stream << std::endl;
		dest.begin(stream_size_type(buffIndex) + (stream?stream->size():stream_size_type(0)), beginData);
		if (!backwards) 
			for (memory_size_type i=0; i < buffIndex; ++i)
				dest.push(buff[i]);
		
		if (stream) {
			{
				source_t source(*stream);
				source.pull_begin();
				while (!source.can_pull())
					dest.push(source.pull());
				source.pull_end();
			}
			delete stream;
			stream = 0;
		}
		
		if (backwards) 
			for (memory_size_type i=buffIndex-1; i != memory_size_type(-1); --i)
				dest.push(buff[i]);
		
		if (buff) {
			delete[] buff;
			buff = 0;
		}
		dest.end(endData);
	}
	
	virtual void memoryNext(std::vector<memory_base *> & next) {
		next.push_back(&dest);
	}
};

}
}

#endif // _TPIE_STREAMING_BUFFER_H
