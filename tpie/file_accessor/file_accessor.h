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
#ifndef __tpie_file_accessor_file_accossor_h__
#define __tpie_file_accessor_file_accossor_h__
#include <tpie/types.h>
#include <tpie/stream_header.h>

namespace tpie {
namespace file_accessor {

class file_accessor {
protected:
	std::string m_path;
	stream_size_type m_size;
public:
	virtual void open(const std::string & path, 
					  bool read, 
					  bool write, 
					  memory_size_type itemSize,
					  memory_size_type userDataSize) = 0;
	virtual void close() = 0;
	virtual memory_size_type read(void * data, stream_size_type offset, memory_size_type size) = 0;
	virtual void write(const void * data, stream_size_type offset, memory_size_type size) = 0; 
	virtual void read_user_data(void * data) = 0;
	virtual void write_user_data(const void * data) = 0;
	virtual ~file_accessor() {}
	inline stream_size_type size() const {
		return m_size;
	}
	inline const std::string & path() const {
		return m_path;
	}
};

}
}
#endif //__tpie_file_accessor_file_accossor_h__
