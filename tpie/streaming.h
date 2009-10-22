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

#ifndef _TPIE_STREAMING_H
#define _TPIE_STREAMING_H
#include <tpie/stream.h>
namespace tpie {
	
	template <class stream_t, class dest_t> 
	class stream_source {
	private:
		stream_t * stream;
		dest_t & dest;
	public:
		stream_source(stream_t * s, dest_t & d): stream(s), dest(d) {};
		inline void run() {
			typename stream_t::item_type * item;
			dest.begin(stream->stream_len());
			while(stream->read_item(&item) != ami::END_OF_STREAM) 
				dest.push(*item);
			dest.end();
		};
	};

	template <class s_t> 
	class stream_sink {
	private:
		s_t * stream;
	public:
		typedef typename s_t::item_type item_type;

		inline stream_sink(s_t * s): stream(s) {}
		inline void begin(TPIE_OS_OFFSET size=0) {}
		inline void push(const item_type & item) {
			stream->write_item(item);
		}
		inline void end() {}
	};
	
}

#endif //_TPIE_STREAMING_H
