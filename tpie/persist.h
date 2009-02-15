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

// Persistence flags for TPIE streams.
#ifndef _TPIE_PERSIST_H
#define _TPIE_PERSIST_H

///////////////////////////////////////////////////////////////////////////
/// \file persist.h Declares persistence tags for TPIE streams.
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

namespace tpie {
	
/** Declares for a stream under which circumstances it should be deleted. */
    enum persistence {
	/** Delete the stream from the disk when it is destructed. */
	PERSIST_DELETE = 0,
	/** Do not delete the stream from the disk when it is destructed. */
	PERSIST_PERSISTENT = 1,
	/** Delete each block of data from the disk as it is read.
	 * If not supported by the OS (see portability.h), delete
	 * the stream when it is destructed (see PERSIST_DELETE). */
	PERSIST_READ_ONCE = TPIE_OS_PERSIST_READ_ONCE
    };

}  //  tpie namespace 

#endif // _TPIE_PERSIST_H 
