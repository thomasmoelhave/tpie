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

#ifndef __TPIE_STREAM_HEADER_H__
#define __TPIE_STREAM_HEADER_H__
#include <boost/cstdint.hpp>
#include <tpie/util.h>

namespace tpie {
namespace stream {

struct header_t {
	static const boost::uint64_t magicConst = 0x521cbe927dd6056all;
	static const size_type reservedCount = 6;
	static const boost::uint64_t versionConst = 1;

	boost::uint64_t magic;
	boost::uint64_t version;
	boost::uint64_t itemSize;
	boost::uint64_t typeMagic;
	boost::uint64_t size;
	boost::uint64_t reserved[reservedCount];
};

}
}
#endif //__TPIE_STREAM_HEADER_H__
