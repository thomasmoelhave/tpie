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

///////////////////////////////////////////////////////////////////////////
/// \namespace tpie::stream The namespace within TPIE streams are defined
/////////////////////////////////////////////////////////////////////////// 

///////////////////////////////////////////////////////////////////////////
/// \file file.h Defines the default file and stream type
/////////////////////////////////////////////////////////////////////////// 

#ifndef __TPIE_FILE_STREAM_H__
#define __TPIE_FILE_STREAM_H__
#include <tpie/file.h>
namespace tpie {

template <typename T>
class file_stream {
public:
	typedef T item_type;
	typedef file<T> file_type;
	typedef typename file<T>::stream stream_type;
private:
	file_type m_file;
	stream_type m_stream;
public:
	inline file_stream(float blockFactor=1.0, file_accessor::file_accessor * fileAccessor=NULL):
		m_file(blockFactor, fileAccessor), m_stream(m_file, 0) {};

	inline void open(const std::string & path, file_base::access_type accessType, memory_size_type user_data_size = 0) {
		m_file.open(path, accessType, user_data_size);
	}

	template <typename TT>
	void read_user_data(TT & data) {
		m_file.read_user_data(data);
	}

	template <typename TT>
	void write_user_data(const TT & data) {
		m_file.write_user_data(data);
	}

	inline void close() {
		m_stream.free();
		m_file.close();
	}
	
	inline void write(const item_type & item) {
		m_stream.write(item);
	}

	template <typename IT>
	inline void write(const IT & start, const IT & end) {
		m_stream.write(start, end);
	}

	inline item_type & read() {
		return m_stream.read();
	}
	
	template <typename IT>
	inline void read(const IT & start, const IT & end) {
		m_stream.read(start, end);
	}

	inline stream_size_type offset() const {
		return m_stream.offset();
	}

	inline const std::string & path() const {
		return m_file.path();
	}

	inline stream_size_type size() const {
		return m_stream.size();
	}

	inline bool has_more() const {
		return m_stream.has_more();
	}

	inline void seek(stream_size_type offset) {
		return m_stream.seek(offset);
	}

	inline static memory_size_type memory_usage(memory_size_type count=1, float bf=1.0, bool includeDefaultFileAccessor=true) {
		return file_type::memory_usage(count, includeDefaultFileAccessor) + stream_type::memory_usage(count, bf);
	}
	
	inline void truncate(stream_size_type size) {
		m_file.truncate(size);
	}
};
}

#endif //__TPIE_FILE_STREAM_H__
