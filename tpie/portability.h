// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2012, The TPIE development team
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
/// \file portability.h
/// This file contains a few deprecated definitions for legacy code.
///////////////////////////////////////////////////////////////////////////////

// This header-file offers macros for independent use on Win and Unix systems.
#ifndef _PORTABILITY_H
#define _PORTABILITY_H

#include <tpie/types.h>
#include <tpie/err.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX  //ensures that the windows min/max macros are not defined 
#endif
#endif

#ifndef _WIN32
#include <sys/times.h>				
#include <sys/resource.h>	
#endif 

#ifdef _WIN32
typedef int TPIE_OS_TMS;
#else
typedef tms TPIE_OS_TMS;
#endif

#ifdef _WIN32
typedef __int64 TPIE_OS_OFFSET;
#else
typedef off_t TPIE_OS_OFFSET;
#endif	

#if defined (_WIN32) && !defined(__MINGW32__)
typedef SSIZE_T TPIE_OS_SSIZE_T;
#ifdef _TPIE_SMALL_MAIN_MEMORY
#if (_MSC_VER < 1400)
typedef unsigned __int32 TPIE_OS_SIZE_T;
#else
typedef size_t TPIE_OS_SIZE_T;
#endif
#else
typedef size_t TPIE_OS_SIZE_T;
#endif
#else
typedef ssize_t TPIE_OS_SSIZE_T;
typedef size_t TPIE_OS_SIZE_T;
#endif

typedef TPIE_OS_SSIZE_T TPIE_OS_OUTPUT_SIZE_T;

#if defined (_WIN32) && !defined(__MINGW32__)
typedef long TPIE_OS_LONG;
typedef __int64 TPIE_OS_LONGLONG;
typedef unsigned __int64 TPIE_OS_ULONGLONG;
#else
typedef long TPIE_OS_LONG;
typedef long long int TPIE_OS_LONGLONG;
typedef unsigned long long int TPIE_OS_ULONGLONG;
#endif 

#endif 
// _portability_H  //
