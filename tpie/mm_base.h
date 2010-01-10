// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
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

#ifndef _TPIE_MEM_MM_BASE_H
#define _TPIE_MEM_MM_BASE_H

///////////////////////////////////////////////////////////////////////////
/// \file mm_base.h 
/// Enum types and superclass declarations for memory management.
///////////////////////////////////////////////////////////////////////////

#include <tpie/config.h>

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>
#include <stdexcept>


namespace tpie {

	#ifdef TPIE_USE_EXCEPTIONS
	struct out_of_memory_error : public std::runtime_error {
		out_of_memory_error(const std::string& s) : std::runtime_error(s) { }
	};
	#endif

    namespace mem {
    
/** MM accounting modes */
        enum mode {
	    IGNORE_MEMORY_EXCEEDED=0,
	    ABORT_ON_MEMORY_EXCEEDED,
	    WARN_ON_MEMORY_EXCEEDED
	};
	
/** MM Error codes */
	enum err {
	    NO_ERROR = 0,
	    INSUFFICIENT_SPACE,
	    EXCESSIVE_DEALLOCATION,
	    EXCESSIVE_ALLOCATION
	};
	
/** Types of memory usage queries we can make on streams (either BTE or MM) */
	enum stream_usage {
	    /** Overhead of the object without the buffer */
	    STREAM_USAGE_OVERHEAD = 1,
	    /** Max amount ever used by a buffer */
	    STREAM_USAGE_BUFFER,
	    /** Amount currently in use. */
	    STREAM_USAGE_CURRENT,
	    /** Max amount that will ever be used. */
	    STREAM_USAGE_MAXIMUM,
	    /** Maximum additional amount used by each substream created. */
	    STREAM_USAGE_SUBSTREAM
	};
	
    }  //  mem namespace

}  //  tpie namespace


namespace tpie {

    namespace mem {

        ///////////////////////////////////////////////////////////////////////////////
	/// Originally intended to be the base class for pointers into memory being 
	/// managed by memory managers.  In a uniprocessor, these objects will simply 
	/// contain pointers.  In multiprocessors, they will be more complicated
	/// descriptions of the layout of memory.
	/// \deprecated Currently not used. 
	///////////////////////////////////////////////////////////////////////////////
	class ptr_base{

	public:
	    ///////////////////////////////////////////////////////////////////////////
	    /// This should return 1 to indicate a valid pointer and 0 to
	    /// indicate an invalid one.  It is useful for tests and assertions.
	    ///////////////////////////////////////////////////////////////////////////
	    virtual operator int (void) = 0;
	    
	    virtual ~ptr_base();
	};

    }  //  mem namespace

}  //  tpie namespace

namespace tpie {

    namespace mem {
	
    ///////////////////////////////////////////////////////////////////////////////
    /// Originially intended to be a base class for all memory management objects.
    /// \deprecated Currently not used. 
    ///////////////////////////////////////////////////////////////////////////////
    
    class manager_base {
    public:
	
	///////////////////////////////////////////////////////////////////////////
	/// Returns how much memory (in bytes) is currently available to TPIE.
	///////////////////////////////////////////////////////////////////////////
	virtual err available (memory_size_type *sz_a) = 0;
	
	virtual ~manager_base();       
    };
	
    }  //  mem namespace

}  //   tpie namespace

#endif // _TPIE_MEM_MM_BASE_H 
