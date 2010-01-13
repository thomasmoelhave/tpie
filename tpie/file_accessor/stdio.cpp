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
#include <tpie/config.h>
//#include <tpie/stream/stdio_bte.h>
//#include <tpie/stream/header.h>
#include <tpie/mm_base.h>
#include <tpie/mm_manager.h>
#include <tpie/exception.h>
#include <string.h>

#include <tpie/file_accessor/stdio.h>
namespace tpie {
namespace file_accessor {

stdio::stdio():
	m_fd(0) {}

void stdio::read_header() {
	stream_header_t header;

	if (::fseeko(m_fd, 0, SEEK_SET) != 0)
		throw_errno();

	if (::fread(&header, 1, sizeof(header), m_fd) != sizeof(header))
		throw_errno();

	if (header.magic != stream_header_t::magicConst)
		throw invalid_file_exception("Invalid file, header magic wrong");

	if (header.version != stream_header_t::versionConst)
		throw invalid_file_exception("Invalid file, header version wrong");
	
	if (header.itemSize != m_itemSize)
		throw invalid_file_exception("Invalid file, item size is wrong");

	if (header.userDataSize != m_userDataSize )
		throw invalid_file_exception("Invalid file, wrong userdata size"); 

	if (header.cleanClose != 1 )
		throw invalid_file_exception("Invalid file, the file was not closed properly"); 

	m_size = header.size;
}

void stdio::throw_errno() {
	throw io_exception(strerror(errno));
}

void stdio::write_header(bool clean) {
	stream_header_t header;
	header.magic = stream_header_t::magicConst;
	header.version = stream_header_t::versionConst;
	header.itemSize = m_itemSize;
	header.cleanClose = clean?1:0;
	header.userDataSize = m_userDataSize;
	header.size = m_size;
	for (memory_size_type i=0; i < stream_header_t::reservedCount; ++ i)
		header.reserved[i] = 0;
	if (::fseeko(m_fd, 0, SEEK_SET) != 0) throw_errno();
	if (::fwrite(&header, 1, sizeof(header), m_fd) != sizeof(header)) throw_errno();
}

void stdio::open(const std::string & path, 
				 bool read, 
				 bool write, 
				 memory_size_type itemSize,
				 memory_size_type userDataSize) {
	close();
	m_path = path;
	m_itemSize=itemSize;
	m_userDataSize=userDataSize;
	m_read = read;
	m_write = write;
	if (!write && !read)
		throw invalid_argument_exception("Either read or write must be specified");
	if (write && !read) {
		m_fd = ::fopen(path.c_str(), "wb");
		if (m_fd == 0) throw_errno();
		m_size = 0;
		write_header(true);
	} else if (!write && read) {
		m_fd = ::fopen(path.c_str(), "rb");
		if (m_fd == 0) throw_errno();
		read_header();
	} else {
		m_fd = ::fopen(path.c_str(), "r+b");
		if (m_fd == 0) {
			if (errno != ENOENT) throw_errno();
			m_fd = ::fopen(path.c_str(), "w+b");
			if (m_fd == 0) throw_errno();
			m_size=0;
			write_header(false);
		} else {
			read_header();
			write_header(false);
		}
	}
	setvbuf(m_fd, NULL, _IONBF, 0);
}

void stdio::close() {
	if (m_fd && m_write) write_header(true);
	if (m_fd != 0) ::fclose(m_fd);
	m_fd=0;
}

memory_size_type stdio::read(void * data, stream_size_type offset, memory_size_type size) {
	stream_size_type loc=sizeof(stream_header_t) + m_userDataSize + offset*m_itemSize;
	if (::fseeko(m_fd, loc, SEEK_SET) != 0) throw_errno();
	if (offset + size > m_size) size = m_size - offset;
	memory_size_type z=size*m_itemSize;
	if (::fread(data, 1, z, m_fd) != z) throw_errno();
	return size;
}

void stdio::write(const void * data, stream_size_type offset, memory_size_type size) {
	stream_size_type loc=sizeof(stream_header_t) + m_userDataSize + offset*m_itemSize;
	if (::fseeko(m_fd, loc, SEEK_SET) != 0) throw_errno();
	memory_size_type z=size*m_itemSize;
	if (::fwrite(data, 1, z, m_fd) != z) throw_errno();
	if (offset+size > m_size) m_size=offset+size;
}

void stdio::read_user_data(void * data) {
	if (::fseeko(m_fd, sizeof(stream_header_t), SEEK_SET) != 0) throw_errno();
	if (::fread(data, 1, m_userDataSize, m_fd) != m_userDataSize) throw_errno();
}

void stdio::write_user_data(const void * data) {
	if (::fseeko(m_fd, sizeof(stream_header_t), SEEK_SET) != 0) throw_errno();
	if (::fwrite(data, 1, m_userDataSize, m_fd) != m_userDataSize) throw_errno();
}

void stdio::truncate(stream_size_type size) {
	//Since there is no reliable way of trunacing a file, we will just fake it
	m_size = size;
}

stdio::~stdio() {
	close();
}


//size_type stdio_block_transfer_engine::memory(size_type count) {
//	return (sizeof(stdio_block_transfer_engine) + sizeof(stdio_block_transfer_engine_p) + MM_manager.space_overhead() + sizeof(FILE)) * count;
//}

}
}
