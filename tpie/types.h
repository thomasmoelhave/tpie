// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2009, 2010, 2012, The TPIE development team
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
/// \file types.h  Standard types.
///////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_TYPES_H
#define _TPIE_TYPES_H

#include <cstdint>
#include <cstddef>

namespace tpie {

using std::uint8_t;
using std::int8_t;
using std::uint16_t;
using std::int16_t;
using std::uint32_t;
using std::int32_t;
using std::uint64_t;
using std::int64_t;

typedef uint64_t stream_size_type;
typedef int64_t stream_offset_type;
typedef std::size_t memory_size_type;
typedef std::ptrdiff_t memory_offset_type;
typedef stream_offset_type offset_type;
typedef std::size_t size_type;
typedef std::ptrdiff_t ssize_type;

}

#endif //_TPIE_TYPES_H
