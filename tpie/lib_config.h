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

#ifndef _LIB_CONFIG_H
#define _LIB_CONFIG_H

#include <tpie/config.h>

// Use logs if requested.
#if TP_LOG_LIB
#define TPL_LOGGING 1
#endif
#include <tpie/tpie_log.h>

// Enable assertions if requested.
#if TP_ASSERT_LIB
#define DEBUG_ASSERTIONS 1
#endif
#include <tpie/tpie_assert.h>


#endif // _LIB_CONFIG_H 
 
