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
#include <tpie/portability.h>

#include <string.h>
#include <tpie/exception.h>
#include <tpie/file_count.h>
#include <tpie/file_accessor/win32.h>
#include <tpie/file_accessor/file_accessor_crtp.inl>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>

namespace tpie {
namespace file_accessor {

void throw_getlasterror() {
	char buffer[1024];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, buffer, 1023, 0);
	throw io_exception(buffer);
};


win32::win32(): m_fd(INVALID_HANDLE_VALUE) {
	invalidateLocation();
}

inline void win32::read_i(void * data, memory_size_type size) {
	DWORD bytesRead = 0;
	if (!ReadFile(m_fd, data, (DWORD)size, &bytesRead, 0) || bytesRead != size) throw_getlasterror();
}

inline void win32::write_i(const void * data, memory_size_type size) {
	DWORD bytesWritten = 0;
	if (!WriteFile(m_fd, data, (DWORD)size, &bytesWritten, 0) || bytesWritten != size ) throw_getlasterror();
}

inline void win32::seek_i(stream_size_type size) {
	LARGE_INTEGER i;
	i.QuadPart = size;
	if (!SetFilePointerEx(m_fd, i, NULL, 0)) throw_getlasterror();
}

void win32::open(const std::string & path,
				 bool read,
				 bool write,
				 memory_size_type itemSize,
				 memory_size_type userDataSize) {
	DWORD creation_flag = FILE_FLAG_SEQUENTIAL_SCAN;
	DWORD shared_flags = FILE_SHARE_READ | FILE_SHARE_WRITE;
	close();
	invalidateLocation();
	m_write = write;
	m_path = path;
	m_itemSize=itemSize;
	m_userDataSize=userDataSize;
	if (!write && !read)
		throw invalid_argument_exception("Either read or write must be specified");
	if (write && !read) {
		m_fd = CreateFile(path.c_str(), GENERIC_WRITE, shared_flags, 0, CREATE_ALWAYS, creation_flag, 0);
		if (m_fd == INVALID_HANDLE_VALUE) throw_getlasterror();
		m_size = 0;
		write_header(false);
		char * buf = new char[userDataSize];
		write_user_data(buf);
		delete[] buf;
	} else if (!write && read) {
		m_fd = CreateFile(path.c_str(), GENERIC_READ, shared_flags, 0, OPEN_EXISTING, creation_flag, 0);
		if (m_fd == INVALID_HANDLE_VALUE) throw_getlasterror();
		read_header();
	} else {
		m_fd = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE , shared_flags, 0, OPEN_EXISTING, creation_flag, 0);
		if (m_fd == INVALID_HANDLE_VALUE) {
			if (GetLastError() != ERROR_FILE_NOT_FOUND) throw_getlasterror();
			m_fd = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE , shared_flags, 0, CREATE_NEW, creation_flag, 0);
			if (m_fd == INVALID_HANDLE_VALUE) throw_getlasterror();
			m_size=0;
			write_header(false);
			char * buf = new char[userDataSize];
			write_user_data(buf);
			delete[] buf;
		} else {
			read_header();
			write_header(false);
		}
	}
	increment_open_file_count();
}

void win32::close() {
	if (m_fd != INVALID_HANDLE_VALUE && m_write) write_header(true);
	if (m_fd != INVALID_HANDLE_VALUE) {
		CloseHandle(m_fd);
		decrement_open_file_count();
	}
	m_fd=INVALID_HANDLE_VALUE;
}

void win32::truncate(stream_size_type size) {
	LARGE_INTEGER i;
	i.QuadPart = size;
	SetFilePointerEx(m_fd, i, NULL, 0);
	if (!SetEndOfFile(m_fd)) throw_getlasterror();
	invalidateLocation();
}

}
}
