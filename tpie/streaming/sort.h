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

namespace tpie {
namespace streaming {

template <typename T>
struct key_identity {
	inline const T & operator()(const T & item) const {
		return item;
	}
};

template <typename item_t, typename comp_t,
		  typename key_t, typename super_t>
class sort_base: public memory_split {
public:
	typedef item_t item_type;
protected:
	typedef std::pair<item_type, TPIE_OS_SIZE_T> queue_item;
	
	struct icomp_t {
		comp_t comp;
		key_t key;
		icomp_t(comp_t & c, key_t & k): comp(c), key(k) {};
		bool operator()(const item_type & a, const item_type & b) const {
			return comp(key(a), key(b));
		}
	};
	
	icomp_t comp;
	item_type * buffer;
	
	TPIE_OS_SIZE_T bufferSize;
	TPIE_OS_SIZE_T bufferIndex;
	TPIE_OS_OFFSET size;
	
	std::string file_base;
	TPIE_OS_SIZE_T next_file;

	TPIE_OS_SIZE_T baseMinMem() {
		return sizeof(super_t) + file_base.capacity() + stream_sink< ami::stream<item_type> >::memory();
	}

public:
	sort_base(comp_t c=comp_t(), key_t k=key_t()):
		comp(c,k) {
		bufferSize = 12;
		file_base = tempname::tpie_name("ssort");
	};
	
	TPIE_OS_SIZE_T miminumInMemony() {
		return baseMinMem();
	}

	TPIE_OS_SIZE_T miminumOutMemony() {
		return baseMinMem() + pull_stream_source< ami::stream<item_type> >::memory() * 2;
	}
	
	void begin(TPIE_OS_OFFSET size=0) {
		bufferSize = (min(memoryIn(), memoryOut()) - miminumInMemony()) / sizeof(item_t);
		if (size > 0 && size <= bufferSize)
			buffer = new item_type[size];
		else
			buffer = new item_type[bufferSize];
		bufferIndex=0;
		size=0;
		next_file = 0;
	}
	
	std::string name(TPIE_OS_SIZE_T number) {
		std::ostringstream ss;
		ss << file_base << "_" << number;
		return ss.str();
	}


	void flush() {
		ami::stream<item_type> stream( name(next_file) );
		stream_sink<ami::stream<item_type> > sink(&stream);
		sink.begin(bufferIndex);
		
		item_type * end=buffer+bufferIndex;
		std::sort(buffer, end, comp);
		for(item_type * item=buffer; item != end; ++item)
			sink.push(*item);
		size += bufferIndex;
		bufferIndex = 0;

		++next_file;
	}
	
	struct qcomp_t: public std::binary_function<queue_item, queue_item, bool > {
		icomp_t comp;
		qcomp_t (icomp_t & _): comp(_) {}
		inline bool operator()(const queue_item & a, const queue_item & b) const {
			return !comp(a.first, b.first);
		}
	};

	class merger {

	};
	
template <class T>
	
	
// 	template <class T> 
// 	void merge(TPIE_OS_SIZE_T count, T & out) {
// 		ami::stream<item_type> ** streams = new ami::stream<item_type> *[count];
		
// 		qcomp_t c(comp);
// 		std::priority_queue<queue_item, std::vector<queue_item>, qcomp_t> queue(c);
// 		for (int i=0; i < count; ++i) {
// 			streams[i] = new ami::stream<item_type>(files.front());
// 			files.pop();
// 			item_type * item;
// 			if (streams[i]->read_item(&item) != ami::END_OF_STREAM)
// 				queue.push( make_pair(*item, i) );
// 		}
		
// 		while (!queue.empty()) {
// 			queue_item p = queue.top();
// 			queue.pop();
// 			out.push(p.first);
			
// 			item_type * item;
// 			if (streams[p.second]->read_item(&item) != ami::END_OF_STREAM)
// 				queue.push( make_pair(*item, p.second) );
// 		}
		
		
// 		for (int i=0; i < count; ++i)
// 			delete streams[i];
// 		delete[] streams;
// 	}
	
	inline void push(const item_type & item) {
		if (bufferIndex >= bufferSize) flush();
		buffer[bufferIndex++] = item;
	}
};


template <class item_t,
		  class comp_t=std::less<item_t>,
		  class key_t=key_identity<item_t>
		  >
class pull_sort: public sort_base<item_t, comp_t, key_t, pull_sort<item_t, comp_t, key_t> > {
private:
	typedef sort_base<item_t, comp_t, key_t, pull_sort<item_t, comp_t, key_t> > parent_t;
	using parent_t::nextFile;
	using parent_t::firstFile;
	using parent_t::bufferIndex;
	using parent_t::buffer;
	using parent_t::comp;
	using parent_t::flush;
	using parent_t::memoryOut;
	using parent_t::baseMinMem;
	using parent_t::baseMerge;
	typename parent_t::Merger * merger;
	TPIE_OS_SIZE_T index;

public:
	typedef item_t item_type;

	pull_sort(comp_t c=comp_t(), key_t k=key_t()): parent_t(c, k) {};

	void end() {
		if (nextFile == 0) {
			item_type * end = buffer+bufferIndex;
			std::sort(buffer, end, comp);
			index=0;
			return;
		}
		flush();
		delete[] buffer;
		buffer = NULL;
		TPIE_OS_SIZE_T arity = memory_fits< pull_stream_source< ami::stream<item_type> > >::fits(memoryOut() - baseMinMem());
		baseMerge(arity);
		merger = new typename parent_t::Merger(nextFile-firstFile);
	}

	inline const item_type & pull() {
		if (nextFile == 0)
			return buffer[index++];
	}


	inline bool atEnd() {
		if (nextFile == 0) 
			return index == bufferIndex;
		else
			return merger->atEnd();
	}

	void free() {
		if (buffer) delete[] buffer;
		buffer = NULL;
		if (merger) delete merger;
		merger = NULL;
	}
};

template <class dest_t,
		  class comp_t=std::less<typename dest_t::item_type>,
		  class key_t=key_identity<typename dest_t::item_type>
		  >
class sort: public sort_base<typename dest_t::item_type, comp_t, key_t, sort<dest_t, comp_t, key_t> > {
private:
	typedef typename dest_t::item_type item_type;
	typedef sort_base<item_type, comp_t, key_t, sort<dest_t, comp_t, key_t> > parent_t;
	using parent_t::baseMerge;
	using parent_t::baseMinMem;
	using parent_t::buffer;
	using parent_t::bufferIndex;
	using parent_t::comp;
	using parent_t::firstFile;
	using parent_t::flush;
	using parent_t::memoryOut;
	using parent_t::merge;
	using parent_t::nextFile;
	using parent_t::size;
	dest_t & dest;
public:
	sort(dest_t & d, comp_t c=comp_t(), key_t k=key_t()): parent_t(c, k), dest(d) {};


	void end() {
		if (nextFile == 0) {
			item_type * end = buffer+bufferIndex;
			std::sort(buffer, end, comp);
			dest.begin(bufferIndex);		
			for(item_type * item=buffer; item != end; ++item)
				dest.push(*item);
			dest.end();
			delete[] buffer;
			return;
		}
		flush();
		delete[] buffer;
		buffer = NULL;

		TPIE_OS_SIZE_T arity = memory_fits< pull_stream_source< ami::stream<item_type> > >::fits(memoryOut() - baseMinMem());
		baseMerge(arity);
		
		dest.begin(size);
 		merge(nextFile-firstFile, dest);
		dest.end();
	}


// 		flush();
// 		delete[] buffer;

// 		TPIE_OS_SIZE_T highfanout = stream_source<ami::stream<item_type> >::fits(MM_manager.consecutive_memory_available() - minimumMemory())+2;
// 		TPIE_OS_SIZE_T lowfanout = stream_source<ami::stream<item_type> >::fits(memory() - minimumMemory())+2;
		
// 		while(files.size() > lowfanout) {

// 		}

		
// 		while(files.size() > fanout) {
// 			std::string tmp = tempname::tpie_name("ssort");
			
// 			ami::stream<item_type> stream(tmp);
// 			stream_sink<ami::stream<item_type> > sink(&stream);
// 			sink.begin();
// 			merge(fanout, sink);
// 			sink.end();
// 			files.push(tmp);
// 		}
		
// 	}
	

};

}
}
#endif //_TPIE_STREAMING_SORT_H
