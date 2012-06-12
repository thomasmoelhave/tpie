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
#include <tpie/stats.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <limits>
#include <tpie/tpie_log.h>

namespace tpie {
namespace file_accessor {

template <typename child_t, bool minimizeSeeks>
inline void file_accessor_crtp<child_t, minimizeSeeks>::read_i(void * d, memory_size_type size) {
	reinterpret_cast<child_t*>(this)->read_i(d, size);
	increment_bytes_read(size);
	if (minimizeSeeks) location += size;
}

template <typename child_t, bool minimizeSeeks>
inline void file_accessor_crtp<child_t, minimizeSeeks>::write_i(const void * d, memory_size_type size) {
	reinterpret_cast<child_t*>(this)->write_i(d, size);
	increment_bytes_written(size);
	if (minimizeSeeks) location += size;
}

template <typename child_t, bool minimizeSeeks>
inline void file_accessor_crtp<child_t, minimizeSeeks>::seek_i(stream_size_type loc) {
	if (!minimizeSeeks || location != loc) reinterpret_cast<child_t*>(this)->seek_i(loc);
	if (minimizeSeeks) location = loc;
}

template <typename child_t, bool minimizeSeeks>
inline void file_accessor_crtp<child_t, minimizeSeeks>::open_wo(const std::string & path) {
	self().open_wo(path);
}

template <typename child_t, bool minimizeSeeks>
inline void file_accessor_crtp<child_t, minimizeSeeks>::open_ro(const std::string & path) {
	self().open_ro(path);
}

template <typename child_t, bool minimizeSeeks>
inline bool file_accessor_crtp<child_t, minimizeSeeks>::try_open_rw(const std::string & path) {
	return self().try_open_rw(path);
}

template <typename child_t, bool minimizeSeeks>
inline void file_accessor_crtp<child_t, minimizeSeeks>::open_rw_new(const std::string & path) {
	self().open_rw_new(path);
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
	if (errno == ENOSPC) throw out_of_space_exception(strerror(errno));
	else throw io_exception(strerror(errno));
}

template <typename child_t, bool minimizeSeeks>
void file_accessor_crtp<child_t, minimizeSeeks>::write_header(bool clean) {
	stream_header_t header;
	fill_header(header, clean);
	seek_i(0);
	char * header_area = new char[header_size()];
	memcpy(header_area, &header, sizeof(header));
	memset(header_area+sizeof(header), 0, header_size()-sizeof(header));
	write_i(header_area, header_size());
	delete[] header_area;
}
 
template <typename child_t, bool minimizeSeeks>
memory_size_type file_accessor_crtp<child_t, minimizeSeeks>::read_block(void * data, stream_size_type blockNumber, memory_size_type itemCount) {
	stream_size_type loc = header_size() + blockNumber*m_blockSize;
	seek_i(loc);
	stream_size_type offset = blockNumber*m_blockItems;
	if (offset + itemCount > m_size) itemCount = static_cast<memory_size_type>(m_size - offset);
	memory_size_type z=itemCount*m_itemSize;
	read_i(data, z);
	return itemCount;
}

template <typename child_t, bool minimizeSeeks>
void file_accessor_crtp<child_t, minimizeSeeks>::write_block(const void * data, stream_size_type blockNumber, memory_size_type itemCount) {
	stream_size_type loc = header_size() + blockNumber*m_blockSize;
	seek_i(loc);
	stream_size_type offset = blockNumber*m_blockItems;
	memory_size_type z=itemCount*m_itemSize;
	write_i(data, z);
	if (offset+itemCount > m_size) m_size=offset+itemCount;
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

template <typename child_t, bool minimizeSeeks>
void file_accessor_crtp<child_t, minimizeSeeks>::validate_header(const stream_header_t & header) {
	if (header.magic != stream_header_t::magicConst)
		throw invalid_file_exception("Invalid file, header magic wrong");

	if (header.version != stream_header_t::versionConst)
		throw invalid_file_exception("Invalid file, header version wrong");

	if (header.itemSize != m_itemSize)
		throw invalid_file_exception("Invalid file, item size is wrong");

	if (header.blockSize != m_blockSize)
		throw invalid_file_exception("Invalid file, item size is wrong");

	if (header.userDataSize != m_userDataSize )
		throw invalid_file_exception("Invalid file, wrong userdata size");

	if (header.cleanClose != 1 )
		throw invalid_file_exception("Invalid file, the file was not closed properly");
}

template <typename child_t, bool minimizeSeeks>
void file_accessor_crtp<child_t, minimizeSeeks>::fill_header(stream_header_t & header, bool clean) {
	header.magic = stream_header_t::magicConst;
	header.version = stream_header_t::versionConst;
	header.itemSize = m_itemSize;
	header.blockSize = m_blockSize;
	header.cleanClose = clean?1:0;
	header.userDataSize = m_userDataSize;
	header.size = m_size;
}

template <typename child_t, bool minimizeSeeks>
void file_accessor_crtp<child_t, minimizeSeeks>::open(const std::string & path,
													  bool read,
													  bool write,
													  memory_size_type itemSize,
													  memory_size_type blockSize,
													  memory_size_type userDataSize) {
	if (write)
		TP_LOG_WARNING_ID("Open called for writing");
	else
		TP_LOG_WARNING_ID("Open called for reading");
	close();
	invalidateLocation();
	m_write = write;
	m_path = path;
	m_itemSize=itemSize;
	m_blockSize=blockSize;
	m_blockItems=blockSize/itemSize;
	m_userDataSize=userDataSize;
	m_size=0;
	if (!write && !read)
		throw invalid_argument_exception("Either read or write must be specified");
	if (write && !read) {
		open_wo(path);
		write_header(false);
		if (userDataSize) {
			char * buf = new char[userDataSize];
			write_user_data(buf);
			delete[] buf;
		}
	} else if (!write && read) {
		open_ro(path);
		read_header();
	} else {
		if (!try_open_rw(path)) {
			open_rw_new(path);
			write_header(false);
			if (userDataSize) {
				char * buf = new char[userDataSize];
				write_user_data(buf);
				delete[] buf;
			}
		} else {
			read_header();
			write_header(false);
		}
	}
	increment_open_file_count();
	m_open = true;
}

template <typename child_t, bool minimizeSeeks>
void file_accessor_crtp<child_t, minimizeSeeks>::close() {
	if (!m_open) {
		TP_LOG_WARNING_ID("Close called when not open");
		return;
	}
	TP_LOG_WARNING_ID("Close called");
	if (m_write) {
		TP_LOG_WARNING_ID("Closing properly");
		write_header(true);
	}
	self().close_i();
	decrement_open_file_count();
	m_open = false;
}

}
}
