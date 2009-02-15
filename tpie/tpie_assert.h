// Copyright 2008, The TPIE development team
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

#ifndef _TPIE_ASSERT_H
#define _TPIE_ASSERT_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
#include <iostream>
#include <tpie/tpie_log.h>

namespace tpie {
    
#if DEBUG_ASSERTIONS

#ifdef _WIN32
#pragma warning ( disable : 4127 )
#endif 

#define tp_assert(condition,message) {		\
	if (!((condition) && 1)) {			\
	    TP_LOG_FATAL_ID("Assertion failed:");	\
	    TP_LOG_FATAL_ID(message);			  \
	    std::cerr << "Assertion failed: " << message << "\n";	\
	    assert(condition);						\
	}								\
    }

#else
#define tp_assert(condition,message)
#endif

}  // tpie namespace

#endif // _TPIE_ASSERT_H

