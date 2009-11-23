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

#include <tpie/stream/stdio_bte.h>
#include <tpie/stream/header.h>
#include <tpie/mm_base.h>
#include <tpie/mm_manager.h>
#include <tpie/stream/exception.h>
#include <string.h>

namespace tpie {
namespace stream {

class stdio_block_transfer_engine_p {
public:
	FILE * fd;
	size_t size;
	size_t itemSize;
	boost::uint64_t typeMagic;
	std::string path;
	bool headerDirty;
	bool read;
	bool write;

	stdio_block_transfer_engine_p(bool r, bool w, size_type is, boost::uint64_t tm);

	void read_header();
	void write_header();
	void throw_errno();
};

stdio_block_transfer_engine_p::stdio_block_transfer_engine_p(
	bool r, bool w, size_type is, boost::uint64_t tm) :
	fd(0), size(0), itemSize(is), typeMagic(tm), path(),
	headerDirty(false), read(r), write(w) {}

void stdio_block_transfer_engine_p::read_header() {
	header_t header;

	if (::fseeko(fd, 0, SEEK_SET) != 0) throw_errno();
	if (::fread(&header, 1, sizeof(header), fd) != sizeof(header)) throw_errno();
	if (header.magic != header_t::magicConst ||
		header.version != header_t::versionConst ||
		header.itemSize != itemSize ||
		header.typeMagic != typeMagic) 
		throw invalid_file_exception("Invalid header");
	size = header.size;
}

void stdio_block_transfer_engine_p::throw_errno() {
	throw io_exception(strerror(errno));
}

void stdio_block_transfer_engine_p::write_header() {
	header_t header;
	header.magic = header_t::magicConst;
	header.version = header_t::versionConst;
	header.itemSize = itemSize;
	header.typeMagic = typeMagic;
	header.size = size;
	for(int i=0; i < header_t::reservedCount; ++ i) header.reserved[i]=0;
	if (::fseeko(fd, 0, SEEK_SET) != 0) throw_errno();
	if (::fwrite(&header, 1, sizeof(header), fd) != sizeof(header)) throw_errno();
	headerDirty=false;
}

stdio_block_transfer_engine::stdio_block_transfer_engine(bool read, bool write, size_type itemSize, boost::uint64_t typeMagic) {
	p = new stdio_block_transfer_engine_p(read, write, itemSize, typeMagic);
}

stdio_block_transfer_engine::~stdio_block_transfer_engine() {
	close();
	delete p;
}

void stdio_block_transfer_engine::open(const std::string & path) {
	close();
	p->path = path;
	
	if (!p->write && !p->read) {
		//throw invalid params
	}	
	if (p->write && !p->read) {
		p->fd = ::fopen(path.c_str(), "wb");
		if (p->fd == 0) p->throw_errno();
		p->size = 0;
		p->headerDirty=true;
		p->write_header();
	} else if (!p->write && p->read) {
		p->fd = ::fopen(path.c_str(), "rb");
		if (p->fd == 0) p->throw_errno();
		p->read_header();
	} else {
		p->fd = ::fopen(path.c_str(), "r+b");
		if (p->fd == 0) {
			if (errno != ENOENT) p->throw_errno();
			p->fd = ::fopen(path.c_str(), "w+b");
			if (p->fd == 0) p->throw_errno();
			p->size=0;
			p->headerDirty=true;
			p->write_header();
		} else {
			p->read_header();
		}
	}
	setvbuf(p->fd, NULL, _IONBF, 0);
}

void stdio_block_transfer_engine::open() {
}

void stdio_block_transfer_engine::close() {
	if (p->headerDirty) p->write_header();
	if (p->fd != 0) ::fclose(p->fd);
	p->fd=0;
}

offset_type stdio_block_transfer_engine::size() const {
	return p->size;
}

const std::string & stdio_block_transfer_engine::path() const {
	return p->path;
}

size_type stdio_block_transfer_engine::read(void * data, offset_type offset, size_type size) {
	off_t loc=sizeof(header_t) + offset*p->itemSize;
	if (::fseeko(p->fd, loc, SEEK_SET) != 0) p->throw_errno();
	if (offset + size > p->size) size = p->size - offset;
	size_type z=size*p->itemSize;
	if (::fread(data, 1, z, p->fd) != z) p->throw_errno();
	return size;
}

void stdio_block_transfer_engine::write(void * data, offset_type offset, size_type size) {
	off_t loc=sizeof(header_t) + offset*p->itemSize;
	//if (offset > (offset_type)p->size) 
	//	if (::ftruncate(p->fd, loc) == -1) p->throw_errno();
	if (::fseeko(p->fd, loc, SEEK_SET) != 0) p->throw_errno();
	size_type z=size*p->itemSize;
	if (::fwrite(data, 1, z, p->fd) != z) p->throw_errno();
	if (offset+size > p->size) {
		p->headerDirty=true;
		p->size=offset+size;
	}
}

size_type stdio_block_transfer_engine::memory(size_type count) {
	return (sizeof(stdio_block_transfer_engine) + sizeof(stdio_block_transfer_engine_p) + MM_manager.space_overhead()) * count;
}

}
}
