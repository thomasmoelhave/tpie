// Copyright (C) 2001 Octavian Procopiuc
//
// File:    tpie_stats_coll.h
// Authors: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: tpie_stats_coll.h,v 1.4 2003-04-17 20:05:10 jan Exp $
//
// Statistics for block collections.

#ifndef _TPIE_STATS_COLL_H
#define _TPIE_STATS_COLL_H

///////////////////////////////////////////////////////////////////////////
/// \file tpie_stats_coll.h
/// Enum type declarations for the statistics object for TPIE collections.
///////////////////////////////////////////////////////////////////////////

// Get definitions for working with Unix and Windows
#include <tpie/portability.h>

#include <tpie/tpie_stats.h>

#define TPIE_STATS_COLLECTION_COUNT 9
/** Statistics tags for profiling a) a single collection or b)
 * all collections in an application instance. */
enum TPIE_STATS_COLLECTION {
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
  COLLECTION_DELETE
};

/** Typedef for the statistics object for TPIE collections. */
typedef tpie_stats<TPIE_STATS_COLLECTION_COUNT> tpie_stats_collection;

#endif //_TPIE_STATS_COLL_H
