// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
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

#ifndef __TPIE_PIPELINING_LEGACY_TYPEDEFS_H__
#define __TPIE_PIPELINING_LEGACY_TYPEDEFS_H__

namespace tpie {

namespace pipelining {

#ifdef __GNUC__
typedef node pipe_segment __attribute__ ((deprecated));
typedef node_token segment_token __attribute__ ((deprecated));
#elif defined(_MSC_VER)
typedef node __declspec(deprecated) pipe_segment;
typedef node_token __declspec(deprecated) segment_token;
#else
typedef node pipe_segment;
typedef node_token segment_token;
#endif

} // namespace pipelining

} // namespace tpie

#endif // __TPIE_PIPELINING_LEGACY_TYPEDEFS_H__
