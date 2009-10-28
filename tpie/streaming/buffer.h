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

#ifndef _TPIE_STREAMING_BUFFER_H
#define _TPIE_STREAMING_BUFFER_H
#include <tpie/streaming/stream.h>
#include <tpie/streaming/memory.h>

namespace tpie {
	namespace streaming {
		
		template <class item_t, class super_t>
		class buffer_base: public memory_split {
		public:
			typedef item_t item_type;
		protected:
			std::vector<item_type> items;
		public:
			inline void begin(TPIE_OS_OFFSET size=0) {
				
			}
			
			inline void push(const item_type & item) {
				items.push_back(item);				
			}

			inline void end() {
				static_cast<super_t*>(this)->done();
			}

		};
		
		
		template <class T>
		class pull_buffer: public buffer_base<T, pull_buffer<T> > {
		protected:
			inline void done() {}
		private:
		public:
			virtual TPIE_OS_SIZE_T minimumMemory() {
				return sizeof(pull_buffer) + 1024; //std::max(stream_sink<T>::memony(), stream_source<T>::memory());
			}
		};

		template <class dest_t> 
		class buffer: public buffer_base<typename dest_t::item_type, pull_buffer<dest_t> > {
		private:
			using buffer_base<typename dest_t::item_type, pull_buffer<dest_t> >::items;
			dest_t & dest;
		protected:
			inline void done() {
				dest.begin(items.size());
				for(size_t i=0; i < items.size(); ++i)
					dest.push(items[i]);
				dest.end();
				items.clear();
			}
		public:
			buffer(dest_t & d): dest(d) {};

			virtual TPIE_OS_SIZE_T minimumMemory() {
				return sizeof(buffer) + 1024; //std::max(stream_sink::memony(), stream_source::memory());
			}
			
			virtual void memoryNext(std::vector<memory_base *> & next) {
				next.push_back(&dest);
			}
		};
	}
}


#endif // _TPIE_STREAMING_BUFFER_H
