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

#include <io.h>
#include <windows.h>
#undef NO_ERROR

#include <tpie/file_accessor/file_accessor_crtp.h>
namespace tpie {
namespace file_accessor {

class win32: public file_accessor_crtp<win32> {
private:
	HANDLE m_fd;
	bool m_write;

	friend class file_accessor_crtp<win32>;

	inline void read_i(void * data, memory_size_type size);
	inline void write_i(const void * data, memory_size_type size);
	inline void seek_i(stream_size_type offset);
public:
	inline win32();
	inline void open(const std::string & path,
					 bool read,
					 bool write,
					 memory_size_type itemSize,
					 memory_size_type blockSize,
					 memory_size_type userDataSize);
	inline void close();
	inline void truncate(stream_size_type size);
	inline ~win32() {close();}
};

}
}

#include <tpie/file_accessor/win32.inl>

#endif //_TPIE_FILE_ACCESSOR_WIN32_H
