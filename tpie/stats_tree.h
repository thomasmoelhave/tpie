// Copyright (C) 2001 Octavian Procopiuc
//
// File:    tpie_stats_tree.h
// Author:  Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: tpie_stats_tree.h,v 1.2 2003-04-17 20:07:23 jan Exp $
//
//
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
