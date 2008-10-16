//
// File: bte/stream_base_generic.h (formerly bte/base_stream.h)
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 5/11/94
//
// $Id: bte/stream_base_generic.h,v 1.1 2005-11-10 12:00:00 jan Exp $
//
#ifndef _TPIE_BTE_STREAM_BASE_GENERIC_H
#define _TPIE_BTE_STREAM_BASE_GENERIC_H

// Get definitions for working with Unix and Windows
#include <portability.h>

// Get statistics definitions.
#include <tpie_stats_stream.h>

namespace bte {
    
// A base class for the base class :). The role of this class is to
// provide global variables, accessible by all streams, regardless of
// template.
    class stream_base_generic {
	
    protected:
	static tpie_stats_stream gstats_;
	static int remaining_streams;

    public:
	// The number of globally available streams.
	static int available_streams() { 
	    return remaining_streams; 
	}
	
	// The global stats.
	static const tpie_stats_stream& gstats() { 
	    return gstats_; 
	}
    };

}

#endif // _TPIE_BTE_STREAM_BASE_GENERIC_H 
