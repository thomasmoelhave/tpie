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
#include <tpie/config.h>
#include <string.h>
#include <tpie/exception.h>
#include <tpie/file_count.h>
#include <tpie/file_accessor/posix.h>
#include <tpie/file_accessor/file_accessor_crtp.inl>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>

namespace tpie {
namespace file_accessor {

posix::posix():
	m_fd(0) {
	invalidateLocation();
}

inline void posix::read_i(void * data, memory_size_type size) {
	if (::read(m_fd, data, size) != (memory_offset_type)size) throw_errno();
}

inline void posix::write_i(const void * data, memory_size_type size) {
	if (::write(m_fd, data, size) != (memory_offset_type)size) throw_errno();
}

inline void posix::seek_i(stream_size_type size) {
	if (::lseek(m_fd, size, SEEK_SET) == -1) throw_errno();
}

void posix::open(const std::string & path,
				 bool read,
				 bool write,
				 memory_size_type itemSize,
				 memory_size_type userDataSize) {
	close();
	invalidateLocation();
	m_write = write;
	m_path = path;
	m_itemSize=itemSize;
	m_userDataSize=userDataSize;
	if (!write && !read)
		throw invalid_argument_exception("Either read or write must be specified");
	if (write && !read) {
		m_fd = ::open(path.c_str(), O_WRONLY | O_TRUNC | O_CREAT,  S_IRUSR | S_IWUSR);
		if (m_fd == -1) throw_errno();
		m_size = 0;
		write_header(false);
		char * buf = new char[userDataSize];
		write_user_data(buf);
		delete[] buf;
	} else if (!write && read) {
		m_fd = ::open(path.c_str(), O_RDONLY);
		if (m_fd == -1) throw_errno();
		read_header();
	} else {
		m_fd = ::open(path.c_str(), O_RDWR);
		if (m_fd == -1) {
			if (errno != ENOENT) throw_errno();
			m_fd = ::open(path.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
			if (m_fd == -1) throw_errno();
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

void posix::close() {
	if (m_fd && m_write) write_header(true);
	if (m_fd != 0) {
		::close(m_fd);
		decrement_open_file_count();
	}
	m_fd=0;
}


void posix::truncate(stream_size_type size) {
	if (ftruncate(m_fd, sizeof(stream_header_t) + m_userDataSize + size*m_itemSize) == -1) throw_errno();
	invalidateLocation();
}



}
}
