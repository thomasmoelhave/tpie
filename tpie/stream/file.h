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

///////////////////////////////////////////////////////////////////////////
/// \namespace tpie::stream The namespace within TPIE streams are defined
/////////////////////////////////////////////////////////////////////////// 

///////////////////////////////////////////////////////////////////////////
/// \file file.h Defines the default file and stream type
/////////////////////////////////////////////////////////////////////////// 

#ifndef __TPIE_STREAM_FILE_H__
#define __TPIE_STREAM_FILE_H__
#include <fd_file.h>

namespace tpie {
namespace stream {

///////////////////////////////////////////////////////////////////////////
/// The default implementation of the tpie::stream::concepts::file concept
/////////////////////////////////////////////////////////////////////////// 
template <typename T, bool canRead, bool canWrite, int blockFactor=100>
class file: public fd_file<T, canRead, canWrite, int blockFactor> {
public:
	file(boost::uint64_t typeMagic=0):
		fd_file(typeMagic);
};

}
}
#endif //__TPIE_STREAM_FILE_H__
