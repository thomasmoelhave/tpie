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

#ifndef _TPIE_STREAMING_STREAM_H
#define _TPIE_STREAMING_STREAM_H
#include <tpie/stream.h>
#include <tpie/streaming/memory.h>
#include <tpie/streaming/util.h>
#include <tpie/file.h>

namespace tpie {
namespace streaming {

template <typename item_t, 
		  typename begin_data_t=empty_type, 
		  typename end_data_t=empty_type> 
class stream_sink: public memory_single {
public:
	typedef item_t item_type;
	typedef begin_data_t begin_data_type;
	typedef end_data_t end_data_type;
	typedef typename file<item_type>::stream stream_type;
private:
	stream_type * m_stream;
	begin_data_type * m_beginData;
	end_data_type * m_endData;
public:
	inline stream_sink(stream_type * stream) throw() : 
		m_stream(stream), 
		m_beginData(0), 
		m_endData(0) {}
	
	inline stream_sink(file_stream<item_type> * stream) throw() : 
		m_stream(stream->stream()), 
		m_beginData(0), 
		m_endData(0) {}
	
	inline void begin(stream_size_type items=max_items, 
					  begin_data_type * data=0) throw() {
		m_beginData=data;
	}
	
	inline void push(const item_t & item) throw(stream_exception) {
		m_stream->write(item);		
	}
	
	inline void end(end_data_t * data=0) throw() {
		m_endData=data;
	}

	inline begin_data_type * begin_data() throw () {
		return m_beginData;
	}

	inline end_data_type * end_data() throw () {
		return m_endData;
	}

	virtual memory_size_type base_memory() {
		return sizeof(*this);
	}
};

template <class dest_t>
class stream_source: public common_single<stream_source<dest_t>, dest_t> {
private:
	typedef common_single<stream_source<dest_t>, dest_t> parent_t;
public:
	using typename parent_t::begin_data_type;
	using typename parent_t::end_data_type;
	typedef typename dest_t::item_type item_type;
	typedef typename file<item_type>::stream stream_type;
private:
	stream_type * m_stream;
public:
	inline stream_source(stream_type * stream, dest_t & dest) throw():
		common_single(dest), 
		m_stream(stream) {}
	
	inline stream_source(file_stream<item_type> * stream, dest_t & dest) throw():
		common_single(dest),
		m_stream(stream->get_stream()) {}
	
	inline void process(bool seek=true,
						begin_data_type * beginData=0, 
						end_data_type * endData=0) throw(stream_exception) {
		if (seek) stream->seek(0);
		dest().begin(stream->size() - stream->offset(), beginData);
		while (stream->can_read())
			dest().push(stream->read());
		dest().end(endData);
	}

	inline void process_back(bool seek=true,
							 begin_data_type * beginData=0,
							 end_data_type * endData=0) throw(stream_exception) {
		if (seek) stream->seek(-1, file_base::end);
		dest().begin(stream->offset()+1, beginData);
		while (stream->can_read_back())
			dest().push(stream->read_back());
		dest().end(endData);
	}

};

template <typename item_t, 
		  bool backwards=false, 
		  typename pull_begin_data_t=empty_type,
		  typename pull_end_data_t=empty_type>
class pull_stream_source: public memory_single {
public:
	typedef item_t item_type;
	typedef pull_begin_data_t pull_begin_data_type;
	typedef pull_end_data_t pull_end_data_type;
	typedef typename file<item_type>::stream stream_type;
private:
	stream_type m_stream;
	bool m_seek;
public:
	inline pull_stream_source(stream_type * stream, bool seek=true) throw():
		m_stream(stream), 
		m_seek(seek) {}

	inline pull_stream_source(file_stream<item_t> * stream, bool seek=true) throw():
		m_stream(stream->stream()),
		m_seek(seek) {}

	inline void pull_begin(stream_size_type * size=0,
						   pull_begin_data_type * data=0) throw(stream_exception) {
		if (m_seek) {
			if (backwards) m_stream->seek(-1, file_base::end);
			else m_stream->seek(0, file_base::beginning);
		}
		if (size)
			*size = backwards?m_stream->offset()+1:m_stream->size()-m_stream->offset();
	}
	
	inline bool can_pull() const throw() {
		return backwards?m_stream->can_read_back():m_stream->can_read();
	}
	
	inline const item_type & pull() throw(stream_exception) {
		return backwards?m_stream->read_back():m_stream->read();
	}

	inline void pull_end(pull_end_data_type * data=0) const throw() {
	}

	virtual memory_size_type base_memory() {
		return sizeof(*this);
	}
};
	

}
}
#endif //_TPIE_STREAMING_STREAM_H
