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

///////////////////////////////////////////////////////////////////////////////
/// \file win32.h  Win32 file accessor.
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_FILE_ACCESSOR_WIN32_H
#define _TPIE_FILE_ACCESSOR_WIN32_H

#include <tpie/config.h>

#include <tpie/file_accessor/stream_accessor.h>
namespace tpie {
namespace file_accessor {

///////////////////////////////////////////////////////////////////////////////
/// \brief Win32 file accessor.
///////////////////////////////////////////////////////////////////////////////

class win32 {
private:
	// m_fd is actually a 'HANDLE' as defined by windows.h
	void * m_fd;
	cache_hint m_cacheHint;

public:
	win32();
	~win32() { close_i(); }

	void open_wo(const std::string & path);
	void open_ro(const std::string & path);
	bool try_open_rw(const std::string & path);
	void open_rw_new(const std::string & path);

	void read_i(void * data, memory_size_type size);
	void write_i(const void * data, memory_size_type size);
	void seek_i(stream_size_type offset);
	void close_i();
	void truncate_i(stream_size_type bytes);
	bool is_open() const;

	void set_cache_hint(cache_hint cacheHint) { m_cacheHint = cacheHint; }

private:
	void * create_file(const std::string & fileName, int desiredAccess,
					   int creationDisposition);
};

}
}

#endif //_TPIE_FILE_ACCESSOR_WIN32_H
