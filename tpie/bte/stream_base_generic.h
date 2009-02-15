#ifndef _TPIE_BTE_STREAM_BASE_GENERIC_H
#define _TPIE_BTE_STREAM_BASE_GENERIC_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

// Get statistics definitions.
#include <tpie/stats_stream.h>

namespace tpie {

    namespace bte {
    
// A base class for the base class :). The role of this class is to
// provide global variables, accessible by all streams, regardless of
// template.
	class stream_base_generic {
	
	protected:
	    static stats_stream gstats_;
	    static int remaining_streams;

	public:
	    // The number of globally available streams.
	    static int available_streams() { 
		return remaining_streams; 
	    }
	
	    // The global stats.
	    static const stats_stream& gstats() { 
		return gstats_; 
	    }
	};

    }  //  bte namespace

}  //  tpie namespace

#endif // _TPIE_BTE_STREAM_BASE_GENERIC_H 
