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

#ifndef _TPIE_STREAMING_SORT_H
#define _TPIE_STREAMING_SORT_H

#include <vector>
#include <algorithm>
#include <tpie/streaming/stream.h>
#include <tpie/streaming/memory.h>
#include <iostream>
#include <tpie/streaming/concepts.h>
#include <tpie/mm_manager.h>

namespace tpie {
namespace streaming {

//struct turnament_pq;
//struct heap_pq;

template <typename value_t,
		  typename stream_t,
		  typename comp_t> 
struct stl_heap_pq {
public:
	typedef std::pair<value_t, stream_t> item_t;
	
	struct mcomp_t {
		comp_t c;
		inline bool operator()(const item_t & a, const item_t & b) const {
			return c(a.first, b.first);
		};
	};
	
	typedef std::priority_queue<item_t, std::vector<item_t>, mcomp_t> pq_t;
	pq_t pq;

	inline stream_t min_stream() const {
		return pq.top().second;
	}
	
	inline bool empty() const {
		return pq.empty();
	}

	inline void insert(const value_t & value, const stream_t & stream) {
		pq.push( item_t(value, stream) );
	}

	inline bool has_delete_min_and_insert() const {
		return false;
	}

	inline value_t delete_min_and_insert(const value_t & nw);
	
	inline value_t deleteMin() {
		value_t v = pq.top().first;
		pq.pop();
		return v;
	}
	
	stl_heap_pq(int n) {}
	
};

// template <typename value_t,
// 		  typedef stream_t,
// 		  typename comp_t,> 
// struct stl_rb_pq {
// };

#define MY_SORT_PQ stl_heap_pq

template <typename item_t, typename comp_t, typename begin_data_t>
class sort_base: public memory_split {
public:
	typedef item_t item_type;
	typedef begin_data_t begin_data_type;
protected:
	typedef std::pair<item_type, memory_size_type> queue_item;
	
	comp_t m_comp;
	item_type * buffer;
	memory_size_type bufferSize;
	memory_size_type bufferIndex;
	stream_size_type size;
	
	std::string fileBase;
	memory_size_type firstFile;
	memory_size_type nextFile;
	begin_data_type * beginData;

	double m_blockFactor;
public:
	sort_base(comp_t comp=comp_t(), double blockFactor=1.0):
		m_comp(comp), m_blockFactor(blockFactor) {
		bufferSize = 12;
		fileBase = tempname::tpie_name("ssort");
	};
	
	virtual memory_size_type base_memory() {
		//TODO include allocation overhead
		return fileBase.capacity();
	}
	
	memory_size_type minimum_memory_in() {
		return base_memory() + //Space for our self
			3*MM_manager.space_overhead() +
			stream_sink<item_type>::memory() + //We write via a streams_sink (uses almost no memory)
			file_stream<item_type>::memory_usage(1, m_blockFactor) + //We write to a file_stream (it will use one block of memory)
			file_base::block_size(m_blockFactor); //We need to buffer elements to sort a block before putting it into a file
	}
	
	memory_size_type minimum_memory_out() {
		return base_memory() +
			file_stream<item_type>::memory_usage(2, m_blockFactor) +
			pull_stream_source<item_type>::memory() * 2;
	}
	
	void begin(stream_size_type items=max_items, begin_data_type * data=0) {
		assert(memory_out() >= minimum_memory_out());
		assert(memory_in() >= minimum_memory_in());
		//memory_size_type mem = std::min(memory_in(), memory_out()) - minimum_memory_out() + file_base::block_size(m_blockFactor);
		memory_size_type mem = memory_in() - minimum_memory_in() + file_base::block_size(m_blockFactor);
		//TODO ensure that mem is less then "consecutive_memory_available"
		beginData=data;
		bufferSize = std::min( stream_size_type(mem / sizeof(item_t)), items );
		buffer = new item_type[bufferSize];
		bufferIndex=0;
		size=0;
		firstFile = 0;
		nextFile = 0;
	}
	
	std::string name(memory_size_type number) {
		std::ostringstream ss;
		ss << fileBase << "_" << number;
		return ss.str();
	}
	
	void flush() {
		file_stream<item_type> stream;
		stream.open(name(nextFile));
		stream_sink<item_type> sink(stream);
		sink.begin(bufferIndex);
		
		item_type * end=buffer+bufferIndex;
		std::sort(buffer, end, m_comp);
		for(item_type * item=buffer; item != end; ++item)
			sink.push(*item);
		size += bufferIndex;
		bufferIndex = 0;
		
		++nextFile;
	}
	
	class Merger {
	public:
		struct qcomp_t: public std::binary_function<queue_item, queue_item, bool > {
			comp_t comp;
			qcomp_t (const comp_t & _): comp(_) {}
			inline bool operator()(const queue_item & a, const queue_item & b) const {
				return !comp(a.first, b.first);
			}
		};
		
		qcomp_t comp;
		std::string fileBase;
		memory_size_type first;
		memory_size_type last;
		file_stream<item_type> ** streams;
		pull_stream_source<item_type> ** sources;
		double m_blockFactor;
		item_type item;
		std::priority_queue<queue_item, std::vector<queue_item>, qcomp_t> queue;
		
		std::string name(memory_size_type number) {
			std::ostringstream ss;
			ss << fileBase << "_" << number;
			return ss.str();
		}
		
		Merger(const std::string & fb, 
			   const comp_t & c, 
			   double blockFactor,
			   memory_size_type f, 
			   memory_size_type l)
			: comp(c), fileBase(fb), first(f), last(l), queue(comp), m_blockFactor(blockFactor)
		{
			streams = new file_stream<item_type> *[last-first];
			sources = new pull_stream_source<item_type> *[last-first];
			assert(last-first >= 2);

			for (memory_size_type i=0; i < last-first; ++i) {
				streams[i] = new file_stream<item_type>(m_blockFactor);
				streams[i]->open( name(i+first));
				sources[i] = new pull_stream_source<item_type>( *streams[i] );
				if (sources[i]->can_pull()) 
					queue.push(std::make_pair(sources[i]->pull(), i));
			}
		}
		
		~Merger() {
			for (memory_size_type i=0; i < last-first; ++i) {
				delete sources[i];
				delete streams[i];
				remove(name(i+first).c_str());
			}
			delete[] streams;
			delete[] sources;
		}
		
		inline item_type & pull() {
			queue_item p = queue.top();
			item = p.first;
			queue.pop();
			if (sources[p.second]->can_pull())
 				queue.push(std::make_pair(sources[p.second]->pull(), p.second));
			return item;
		}

		inline bool can_pull() {
			return !queue.empty();
		}
	};
	
	
	template <class T> 
	void merge(memory_size_type first, memory_size_type last, T & out) {
		Merger merger(fileBase, m_comp, m_blockFactor, first, last);
		while (merger.can_pull())
			out.push(merger.pull());
	}
		
	void baseMerge(memory_size_type arity) {
		memory_size_type highArity = 2;// memory_fits< pull_stream_source< ami::stream<item_type> > >::fits(MM_manager.memory_available() - baseMinMem());
		//TODO analasys of what yields the lowest number of IO's is required
		while( nextFile - firstFile > arity ) {
			memory_size_type count = std::min(nextFile - firstFile - arity+1, highArity);
			file_stream<item_type> stream(m_blockFactor);
			stream.open(name(nextFile));
 			stream_sink<item_type> sink(stream);
 			sink.begin();
 			merge(firstFile, firstFile+count, sink);
 			sink.end();
			firstFile += count;
			nextFile += 1;
		}
	}

	
	inline void push(const item_type & item) {
		if (bufferIndex >= bufferSize) flush();
		buffer[bufferIndex++] = item;
	}
};


template <typename item_t,
		  typename comp_t=std::less<item_t>,
		  typename begin_data_t=empty_type,
		  typename end_data_t=empty_type,
		  typename pull_begin_data_t=empty_type,
		  typename pull_end_data_t=empty_type >
class pull_sort: public sort_base<item_t, comp_t, begin_data_t> {
private:
	typedef sort_base<item_t, comp_t, begin_data_t> parent_t;
	using parent_t::nextFile;
	using parent_t::firstFile;
	using parent_t::bufferIndex;
	using parent_t::buffer;
	using parent_t::m_comp;
	using parent_t::flush;
	using parent_t::memory_out;
	using parent_t::size;
	using parent_t::baseMerge;
	using parent_t::m_blockFactor;
	typename parent_t::Merger * merger;
	memory_size_type index;
	end_data_t * endData;
public:
	typedef item_t item_type;
	typedef item_t pull_type;
	typedef begin_data_t begin_data_type;
	typedef end_data_t end_data_type;
	typedef pull_begin_data_t pull_begin_data_type;
	typedef pull_end_data_t pull_end_data_type;

	pull_sort(comp_t comp=comp_t(), double blockFactor=1.0): parent_t(comp, blockFactor) {};

	virtual memory_size_type base_memory() {
		return parent_t::base_memory() + sizeof(*this);
	}

	void end(end_data_type * data=0) {
		endData=data;
		merger=NULL;
		if (nextFile == 0) {
			item_type * end = buffer+bufferIndex;
			std::sort(buffer, end, m_comp);
			index=0;
			return;
		}
		flush();
		delete[] buffer;
		buffer = NULL;
		memory_size_type arity = 2; //memory_fits< pull_stream_source< ami::stream<item_type> > >::fits(memoryOut() - baseMinMem());
		baseMerge(arity);
	}

	inline void pull_begin(stream_size_type * items=0, pull_begin_data_type * data=0) {
		unused(data);
		if (nextFile != 0) {
			if (items) *items=size;
			merger = new typename parent_t::Merger(parent_t::fileBase, m_comp, m_blockFactor, firstFile, nextFile);
		} else {
			if (items) *items=bufferIndex;
		}
	}

	inline const item_type & pull() {
		if (nextFile == 0)
			return buffer[index++];
		else
			return merger->pull();
	}

	inline bool can_pull() {
		if (nextFile == 0) 
			return index != bufferIndex;
		else
			return merger->can_pull();
	}

	inline void pull_end(pull_end_data_type * data=0) {
		unused(data);
		if (buffer) delete[] buffer;
		buffer = 0;
		if (merger) delete merger;
		merger = 0;
	}
};

template <class dest_t,
		  class comp_t=std::less<typename dest_t::item_type>
		  >
class sort: public sort_base<typename dest_t::item_type, comp_t, typename dest_t::begin_data_type> {
public:
	typedef typename dest_t::item_type item_type;
	typedef typename dest_t::begin_data_type begin_data_type;
	typedef typename dest_t::end_data_type end_data_type;
private:
	typedef sort_base<item_type, comp_t, begin_data_type> parent_t;
	using parent_t::baseMerge;
	using parent_t::buffer;
	using parent_t::bufferIndex;
	using parent_t::m_comp;
	using parent_t::firstFile;
	using parent_t::flush;
	using parent_t::memory_out;
	using parent_t::merge;
	using parent_t::nextFile;
	using parent_t::size;
	using parent_t::beginData;

	dest_t & m_dest;
public:
	virtual memory_size_type base_memory() {
		return parent_t::base_memory() + sizeof(*this);
	}

	sort(dest_t & dest, comp_t comp=comp_t(), double blockFactor=1.0): parent_t(comp, blockFactor), m_dest(dest) {};

	void end(end_data_type * endData=0) {
		if (nextFile == 0) {
			item_type * end = buffer+bufferIndex;
			std::sort(buffer, end, m_comp);
			m_dest.begin(bufferIndex, beginData);		
			for(item_type * item=buffer; item != end; ++item)
				m_dest.push(*item);
			delete[] buffer;
			buffer=0;
			m_dest.end(endData);
			return;
		}
		flush();
		delete[] buffer;
		buffer=0;
		//memory_size_type arity = memory_fits<pull_stream_source< ami::stream<item_type> > >::fits(memoryOut() - baseMinMem());
		memory_size_type arity = 2;
		baseMerge(arity);
		m_dest.begin(size, beginData);
 		merge(firstFile, nextFile, m_dest);
		m_dest.end(endData);
	}	

	virtual void memory_next(std::vector<memory_base *> & next) {
		next.push_back(&m_dest);
	}
};

}
}
#endif //_TPIE_STREAMING_SORT_H
