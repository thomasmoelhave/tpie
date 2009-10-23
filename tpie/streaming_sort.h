// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
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

namespace tpie {

	template <class T>
	struct key_identity {
		inline const T & operator()(const T & item) const {
			return item;
		}
	};

	template <class dest_t,
			  class comp_t=std::less<typename dest_t::item_type>,
			  class key_t=key_identity<typename dest_t::item_type> >
	class streaming_sort {
	public:
		typedef typename dest_t::item_type item_type;
	private:
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
		dest_t & dest;
		item_type * buffer;
		
		TPIE_OS_SIZE_T bufferSize;
		TPIE_OS_SIZE_T bufferIndex;
		TPIE_OS_OFFSET size;

		std::queue< std::string > files;
	public:


		streaming_sort(dest_t & d, comp_t c=comp_t(), key_t k=key_t()):
			comp(c,k), dest(d) {
			bufferSize = 12;
		};

		void setMemoryUsage(TPIE_OS_SIZE_T memory) {
			this->bufferSize = (
				memory 
				- 1024 * 10  //Substract the size of a single stream sink (TODO fix)
				- 1024 * 500 //Reserve space to store the names of the temporery files
				- sizeof(streaming_sort))
				/ sizeof(item_type);
		}

		void setMemoryUsage(double f=1.0) {
			setMemoryUsage(1024*1024*50 * f);
		}

		void begin(TPIE_OS_OFFSET size=0) {
			if (size > 0 && size <= bufferSize)
				buffer = new item_type[size];
			else
				buffer = new item_type[bufferSize];
			bufferIndex=0;
			size=0;
		}

		void flush() {
			std::string tmp = tempname::tpie_name("ssort");
			ami::stream<item_type> stream(tmp);
			stream_sink<ami::stream<item_type> > sink(&stream);
			sink.begin(bufferIndex);
			
			item_type * end=buffer+bufferIndex;
			std::sort(buffer, end, comp);
			for(item_type * item=buffer; item != end; ++item)
				sink.push(*item);
			size += bufferIndex;
			bufferIndex = 0;
			files.push(tmp);
		}

		struct qcomp_t: public std::binary_function<queue_item, queue_item, bool > {
			icomp_t comp;
			qcomp_t (icomp_t & _): comp(_) {}
			inline bool operator()(const queue_item & a, const queue_item & b) const {
				return !comp(a.first, b.first);
			}
		};
		

		template <class T> 
		void merge(TPIE_OS_SIZE_T count, T & out) {
			ami::stream<item_type> ** streams = new ami::stream<item_type> *[count];

			qcomp_t c(comp);
			std::priority_queue<queue_item, std::vector<queue_item>, qcomp_t> queue(c);
			for (int i=0; i < count; ++i) {
				streams[i] = new ami::stream<item_type>(files.front());
				files.pop();
				item_type * item;
				if (streams[i]->read_item(&item) != ami::END_OF_STREAM)
					queue.push( make_pair(*item, i) );
			}
			
			while (!queue.empty()) {
				queue_item p = queue.top();
				queue.pop();
				out.push(p.first);
				
				item_type * item;
				if (streams[p.second]->read_item(&item) != ami::END_OF_STREAM)
					queue.push( make_pair(*item, p.second) );
			}
			
			
			for (int i=0; i < count; ++i)
				delete streams[i];
			delete[] streams;
		}
		
		void end() {
			if (files.size() == 0) {
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
			
			TPIE_OS_SIZE_T fanout=1024;
			
			while(files.size() > fanout) {
				std::string tmp = tempname::tpie_name("ssort");
				
				ami::stream<item_type> stream(tmp);
				stream_sink<ami::stream<item_type> > sink(&stream);
				sink.begin();
				merge(fanout, sink);
				sink.end();
				files.push(tmp);
			}
			
			dest.begin(size);
			merge(files.size(), dest);
			dest.end();
		}


		inline void push(const item_type & item) {
			if (bufferIndex >= bufferSize) flush();
			buffer[bufferIndex++] = item;
		}
	};

}



#endif //_TPIE_STREAMING_SORT_H
