// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, 2010, The TPIE development team
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
#ifndef _TPIE_FILE_ACCESSOR_STDIO_H
#define _TPIE_FILE_ACCESSOR_STDIO_H

#include <tpie/file_accessor/file_accessor.h>

namespace tpie {
namespace file_accessor {

#ifdef _WIN32
#define _fseeki64 fseeko
#endif

class stdio: public file_accessor {
private:
	FILE * m_fd;
	memory_size_type m_itemSize;

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
	virtual void truncate(stream_size_type size);
	virtual ~stdio();
	static inline memory_size_type memory_usage(memory_size_type count) {
		return (sizeof(stdio) + 100) * count;
	}
};

}
}
#endif //_TPIE_FILE_ACCESSOR_STDIO_H
