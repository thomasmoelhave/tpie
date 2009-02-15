// BTE error codes, moved here from bte/base_stream.h
#ifndef _TPIE_BTE_ERR_H
#define _TPIE_BTE_ERR_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#ifdef TPIE_USE_EXCEPTIONS
#include <stdexcept>
#endif

namespace tpie {

    namespace bte {

	#ifdef TPIE_USE_EXCEPTIONS
	class out_of_space_error : public std::runtime_error {
		public:
			out_of_space_error(const std::string& what) 
			: std::runtime_error(what) 
			{ ; }
	};
	#endif 
		
	
//
// BTE error codes.
//
	enum err {
	    NO_ERROR = 0,
	    IO_ERROR,
	    END_OF_STREAM,
	    READ_ONLY,
	    OS_ERROR,
	    BASE_METHOD,
	    MEMORY_ERROR,
	    PERMISSION_DENIED,
	    OFFSET_OUT_OF_RANGE,
	    OUT_OF_SPACE,
	    STREAM_IS_SUBSTREAM,
	    WRITE_ONLY,
	    BAD_HEADER,
	    INVALID_PLACEHOLDER
	};
    
    }  //  bte namespace
    
}  //  tpie namespace

#endif // _TPIE_ERR_H
