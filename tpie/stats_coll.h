// Statistics for block collections.

#ifndef _TPIE_STATS_COLL_H
#define _TPIE_STATS_COLL_H

///////////////////////////////////////////////////////////////////////////
/// \file tpie_stats_coll.h
/// Enum type declarations for the statistics object for TPIE collections.
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <tpie/stats.h>

namespace tpie {
    
/** Statistics tags for profiling a) a single collection or b)
 * all collections in an application instance. */
    enum stats_collection_id {
	/** Number of block reads */ 
	BLOCK_GET = 0, 
	/** Number of block writes */ 
	BLOCK_PUT,
	/** Number of block creates */ 
	BLOCK_NEW,
	/** Number of block deletes */ 
	BLOCK_DELETE,
	/** Number of block sync operations */ 
	BLOCK_SYNC,
	/** Number of collection open operations */ 
	COLLECTION_OPEN,
	/** Number of collection close operations */ 
	COLLECTION_CLOSE,
	/** Number of collection create operations */ 
	COLLECTION_CREATE,
	/** Number of collection delete operations */ 
	COLLECTION_DELETE,
	NUMBER_OF_COLLECTION_STATISTICS
    };
    
/** Typedef for the statistics object for TPIE collections. */
    typedef stats<NUMBER_OF_COLLECTION_STATISTICS> stats_collection;

}  //  tpie namespace

#endif //_TPIE_STATS_COLL_H
