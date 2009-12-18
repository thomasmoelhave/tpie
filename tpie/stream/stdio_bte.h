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

#ifndef __TPIE_STREAM_STDIO_BTE_H__
#define __TPIE_STREAM_STDIO_BTE_H__
#include <tpie/stream/header.h>
#include <boost/cstdint.hpp>
#include <string>

namespace tpie {
namespace stream {

class stdio_block_transfer_engine_p;

class stdio_block_transfer_engine {
private:
	stdio_block_transfer_engine_p * p;
public:
	stdio_block_transfer_engine(bool read, bool write, size_type itemSize, boost::uint64_t typeMagic);
	~stdio_block_transfer_engine();
	void open(const std::string & p);
	void close();
	offset_type size() const;
	const std::string & path() const;
	size_type read(void * data, offset_type offset, size_type size);
	void write(void * data, offset_type offset, size_type size);
	static size_type memory(size_type count);
};

}
}
#endif //__TPIE_STREAM_STDIO_BTE_H__
