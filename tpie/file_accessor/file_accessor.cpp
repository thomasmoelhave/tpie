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
#include <tpie/file_accessor/file_accessor.h>
#include <tpie/exception.h>

namespace tpie {
namespace file_accessor {

void file_accessor::validate_header(const stream_header_t & header) {
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
}

void file_accessor::fill_header(stream_header_t & header, bool clean) {
	header.magic = stream_header_t::magicConst;
	header.version = stream_header_t::versionConst;
	header.itemSize = m_itemSize;
	header.cleanClose = clean?1:0;
	header.userDataSize = m_userDataSize;
	header.size = m_size;
	for (memory_size_type i=0; i < stream_header_t::reservedCount; ++ i)
		header.reserved[i] = 0;
}

}
}
