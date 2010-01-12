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
#ifndef __tpie_file_accessor_stdio_h__
#define __tpie_file_accessor_stdio_h__

#include <tpie/file_accessor/file_accessor.h>

namespace tpie {
namespace file_accessor {

class stdio: public file_accessor {
private:
	FILE * m_fd;
	memory_size_type m_itemSize;
	memory_size_type m_userDataSize;
	bool m_read;
	bool m_write;

	void read_header();
	void write_header(bool clean);
	void throw_errno();
	
public:
	stdio();
	virtual void open(const std::string & path, 
					  bool read, 
					  bool write, 
					  memory_size_type itemSize,
					  memory_size_type userDataSize);
	virtual void close();
	virtual memory_size_type read(void * data, stream_size_type offset, memory_size_type size);
	virtual void write(const void * data, stream_size_type offset, memory_size_type size); 
	virtual void read_user_data(void * data);
	virtual void write_user_data(const void * data);
	virtual ~stdio();
};

}
}
#endif //__tpie_file_accessor_stdio_h__
