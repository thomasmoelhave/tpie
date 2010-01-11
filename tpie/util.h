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

#ifndef __TPIE_UTIL_H__
#define __TPIE_UTIL_H__

#include <tpie/types.h>
namespace tpie {

///////////////////////////////////////////////////////////////////////////
/// \brief Ignore an unused variable warning
/// \param x The variable that we are well aware is not beeing useod
///////////////////////////////////////////////////////////////////////////
template <typename T>
inline void unused(const T & x) {(void)x;}

void seed_random(uint32_t seed);
uint32_t random();
void remove(const std::string & path);
bool file_exists(const std::string & path);

#ifdef _WIN32
const char directory_delimiter = '\\';
#else
const char directory_delimiter = '/';
#endif


}
#endif //__TPIE_UTIL_H__
