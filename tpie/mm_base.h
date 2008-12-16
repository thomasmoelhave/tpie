#ifndef _TPIE_MEM_MM_BASE_H
#define _TPIE_MEM_MM_BASE_H

///////////////////////////////////////////////////////////////////////////
/// \file mm_base.h 
/// Enum types and superclass declarations for memory management.
///////////////////////////////////////////////////////////////////////////

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
	virtual err available (TPIE_OS_SIZE_T *sz_a) = 0;
	
	virtual ~manager_base();       
    };
	
    }  //  mem namespace

}  //   tpie namespace

#endif // _TPIE_MEM_MM_BASE_H 
