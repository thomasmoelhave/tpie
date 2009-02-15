////////////////////////////////////////////////////////////////////////////////
/// \file tpie_stats_stream.h Declares status information tags for TPIE Streams
/// \sa tpie_stats, stream#stats()
////////////////////////////////////////////////////////////////////////////////

#ifndef _TPIE_STATS_STREAM_H
#define _TPIE_STATS_STREAM_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <tpie/stats.h>

namespace tpie {
    
/**Status information about  a TPIE stream */
    enum stats_stream_id {
	/** Number of block reads */
	BLOCK_READ = 0,
	/** Number of block writes */
	BLOCK_WRITE,
	/** Number of item reads */
	ITEM_READ,
	/** Number of item writes */ 
	ITEM_WRITE,
	/** Number of item seek operations */ 
	ITEM_SEEK,
	/** Number of stream open operations */ 
	STREAM_OPEN,
	/** Number of stream close operations */ 
	STREAM_CLOSE,
	/** Number of stream create operations */ 
	STREAM_CREATE,
	/** Number of stream delete operations */ 
	STREAM_DELETE,
	/** Number of substream create operations */ 
	SUBSTREAM_CREATE,
	/** Number of substream delete operations */ 
	SUBSTREAM_DELETE,
	NUMBER_OF_STREAM_STATISTICS
    };
    
    ///////////////////////////////////////////////////////////////////////////
    /// Encapsulates statistics about a TPIE stream.
    ///////////////////////////////////////////////////////////////////////////
    typedef stats<NUMBER_OF_STREAM_STATISTICS> stats_stream;

}  //  tpie namespace

#endif //_TPIE_STATS_STREAM_H
