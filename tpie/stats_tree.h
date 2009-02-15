#ifndef _TPIE_STATS_TREE_H
#define _TPIE_STATS_TREE_H

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <tpie/stats.h>

namespace tpie {

    enum stats_tree_id {
	LEAF_FETCH = 0,
	LEAF_RELEASE,
	LEAF_READ,
	LEAF_WRITE,
	LEAF_CREATE,
	LEAF_DELETE,
	LEAF_COUNT,
	NODE_FETCH,
	NODE_RELEASE,
	NODE_READ,
	NODE_WRITE,
	NODE_CREATE,
	NODE_DELETE,
	NODE_COUNT,
	NUMBER_OF_TREE_STATISTICS
    };
    
    typedef stats<NUMBER_OF_TREE_STATISTICS> stats_tree;

}  //  tpie namespace

#endif // _TPIE_STATS_TREE_H
