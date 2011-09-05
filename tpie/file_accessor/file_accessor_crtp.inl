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
#include <tpie/file_accessor/file_accessor_crtp.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits>

namespace tpie {
namespace file_accessor {

template <typename child_t, bool minimizeSeeks>
inline void file_accessor_crtp<child_t, minimizeSeeks>::read_i(void * d, memory_size_type size) {
	reinterpret_cast<child_t*>(this)->read_i(d, size);
	if (minimizeSeeks) location += size;
}

template <typename child_t, bool minimizeSeeks>
inline void file_accessor_crtp<child_t, minimizeSeeks>::write_i(const void * d, memory_size_type size) {
	reinterpret_cast<child_t*>(this)->write_i(d, size);
	if (minimizeSeeks) location += size;
}

template <typename child_t, bool minimizeSeeks>
inline void file_accessor_crtp<child_t, minimizeSeeks>::seek_i(stream_size_type loc) {
	if (!minimizeSeeks || location != loc) reinterpret_cast<child_t*>(this)->seek_i(loc);
	if (minimizeSeeks) location = loc;
}

template <typename child_t, bool minimizeSeeks>
void file_accessor_crtp<child_t, minimizeSeeks>::read_header() {
	stream_header_t header;
	seek_i(0);
	read_i(&header, sizeof(header));
	validate_header(header);
	m_size = header.size;
}

template <typename child_t, bool minimizeSeeks>
inline void file_accessor_crtp<child_t, minimizeSeeks>::throw_errno() {
	throw io_exception(strerror(errno));
}

template <typename child_t, bool minimizeSeeks>
void file_accessor_crtp<child_t, minimizeSeeks>::write_header(bool clean) {
	stream_header_t header;
	fill_header(header, clean);
	seek_i(0);
	write_i(&header, sizeof(header));
}

template <typename child_t, bool minimizeSeeks>
memory_size_type file_accessor_crtp<child_t, minimizeSeeks>::read(void * data, stream_size_type offset, memory_size_type size) {
	stream_size_type loc=sizeof(stream_header_t) + m_userDataSize + offset*m_itemSize;
	seek_i(loc);
	if (offset + size > m_size) size = m_size - offset;
	memory_size_type z=size*m_itemSize;
	read_i(data, z);
	return size;
}

template <typename child_t, bool minimizeSeeks>
void file_accessor_crtp<child_t, minimizeSeeks>::write(const void * data, stream_size_type offset, memory_size_type size) {
	stream_size_type loc=sizeof(stream_header_t) + m_userDataSize + offset*m_itemSize;
	seek_i(loc);
	memory_size_type z=size*m_itemSize;
	write_i(data, z);
	if (offset+size > m_size) m_size=offset+size;
}

template <typename child_t, bool minimizeSeeks>
void file_accessor_crtp<child_t, minimizeSeeks>::read_user_data(void * data) {
	seek_i(sizeof(stream_header_t));
	read_i(data, m_userDataSize);
}

template <typename child_t, bool minimizeSeeks>
void file_accessor_crtp<child_t, minimizeSeeks>::write_user_data(const void * data) {
	seek_i(sizeof(stream_header_t));
	write_i(data,  m_userDataSize);
}
   
template <typename child_t, bool minimizeSeeks>
inline void file_accessor_crtp<child_t, minimizeSeeks>::invalidateLocation() {
	if (minimizeSeeks) location = std::numeric_limits<stream_size_type>::max();
}


}
}
