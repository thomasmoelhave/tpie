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
	
	std::string fileBase;
	TPIE_OS_SIZE_T firstFile;
	TPIE_OS_SIZE_T nextFile;

	TPIE_OS_SIZE_T baseMinMem() {
		return sizeof(super_t) + fileBase.capacity() + stream_sink< ami::stream<item_type> >::memory();
	}

public:
	sort_base(comp_t c=comp_t(), key_t k=key_t()):
		comp(c,k) {
		bufferSize = 12;
		fileBase = tempname::tpie_name("ssort");
	};
	
	TPIE_OS_SIZE_T miminumInMemony() {
		return baseMinMem();
	}

	TPIE_OS_SIZE_T miminumOutMemony() {
		return baseMinMem() + pull_stream_source< ami::stream<item_type> >::memory() * 2;
	}
	
	void begin(TPIE_OS_OFFSET size=0) {
		TPIE_OS_SIZE_T mem = min(memoryIn(), memoryOut()) - miminumInMemony();
		//TODO ensure that mem is less then "consecutive_memory_available"
		bufferSize = mem / sizeof(item_t);
		bufferSize = 3;
		if (size > 0 && size <= bufferSize)
			buffer = new item_type[size];
		else
			buffer = new item_type[bufferSize];
		bufferIndex=0;
		size=0;
		firstFile = 0;
		nextFile = 0;
	}
	
	std::string name(TPIE_OS_SIZE_T number) {
		std::ostringstream ss;
		ss << fileBase << "_" << number;
		return ss.str();
	}


	void flush() {
		ami::stream<item_type> stream( name(nextFile) );
		stream_sink<ami::stream<item_type> > sink(&stream);
		sink.begin(bufferIndex);
		
		item_type * end=buffer+bufferIndex;
		std::sort(buffer, end, comp);
		for(item_type * item=buffer; item != end; ++item)
			sink.push(*item);
		size += bufferIndex;
		bufferIndex = 0;

		++nextFile;
	}
	

	class Merger {
	public:
		struct qcomp_t: public std::binary_function<queue_item, queue_item, bool > {
			icomp_t comp;
			qcomp_t (const icomp_t & _): comp(_) {}
			inline bool operator()(const queue_item & a, const queue_item & b) const {
				return !comp(a.first, b.first);
			}
		};
		
		qcomp_t comp;
		std::string fileBase;
		TPIE_OS_SIZE_T first;
		TPIE_OS_SIZE_T last;
		ami::stream<item_type> ** streams;
		pull_stream_source< ami::stream<item_type> > ** sources;
		item_type item;
		std::priority_queue<queue_item, std::vector<queue_item>, qcomp_t> queue;
		
		std::string name(TPIE_OS_SIZE_T number) {
			std::ostringstream ss;
			ss << fileBase << "_" << number;
			return ss.str();
		}
		
		Merger(const std::string & fb, const icomp_t & c, TPIE_OS_SIZE_T f, TPIE_OS_SIZE_T l)
			: comp(c), fileBase(fb), first(f), last(l), queue(comp)
		{
			streams = new ami::stream<item_type> *[last-first];
			sources = new pull_stream_source< ami::stream<item_type> > *[last-first];

			for (int i=0; i < last-first; ++i) {
				streams[i] = new ami::stream<item_type>( name(i+first) );
				sources[i] = new pull_stream_source< ami::stream<item_type> >( streams[i] );
				item_type * item;
				if (!sources[i]->atEnd()) 
					queue.push( make_pair(sources[i]->pull() , i) );
			}
		}
		
		~Merger() {
			for (int i=0; i < last-first; ++i) {
				sources[i]->free();
				delete streams[i];
				delete sources[i];
				remove(name(i+first).c_str());
			}
			delete[] streams;
			delete[] sources;
		}
		
		inline item_type & pull() {
			queue_item p = queue.top();
			item = p.first;
			queue.pop();
			if (!sources[p.second]->atEnd())
 				queue.push( make_pair(sources[p.second]->pull() , p.second) );
			return item;
		}

		inline bool atEnd() {
			return queue.empty();
		}
	};
	
	
	template <class T> 
	void merge(TPIE_OS_SIZE_T first, TPIE_OS_SIZE_T last, T & out) {
		Merger merger(fileBase, comp, first, last);
		while (!merger.atEnd())
			out.push(merger.pull());
	}
		
	void baseMerge(TPIE_OS_SIZE_T arity) {
		TPIE_OS_SIZE_T highArity  = memory_fits< pull_stream_source< ami::stream<item_type> > >::fits(MM_manager.memory_available() - baseMinMem());
		//TODO analasys of what yields the lowest number of IO's is required
		while( nextFile - firstFile > arity ) {
			TPIE_OS_SIZE_T count = std::min(nextFile - firstFile - arity+1, highArity);
 			ami::stream<item_type> stream(name(nextFile));
 			stream_sink<ami::stream<item_type> > sink(&stream);
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
		merger = new typename parent_t::Merger(parent_t::fileBase, comp, firstFile, nextFile);
	}

	inline void beginPoll() {}

	inline const item_type & pull() {
		if (nextFile == 0)
			return buffer[index++];
		else
			return merger->pull();
	}


	inline bool atEnd() {
		if (nextFile == 0) 
			return index == bufferIndex;
		else
			return merger->atEnd();
	}

	inline void endPool() {
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
		arity=2;
		baseMerge(arity);
		dest.begin(size);
 		merge(firstFile, nextFile, dest);
		dest.end();
	}	

};

}
}
#endif //_TPIE_STREAMING_SORT_H
