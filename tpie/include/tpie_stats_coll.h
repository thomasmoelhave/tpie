// Copyright (C) 2001 Octavian Procopiuc
//
// File:    tpie_stats_coll.h
// Authors: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: tpie_stats_coll.h,v 1.3 2002-01-15 03:06:34 tavi Exp $
//
// Statistics for block collections.

#ifndef _TPIE_STATS_COLL_H
#define _TPIE_STATS_COLL_H

#include <tpie_stats.h>

#define TPIE_STATS_COLLECTION_COUNT 9
enum TPIE_STATS_COLLECTION {
  BLOCK_GET = 0,
  BLOCK_PUT,
  BLOCK_NEW,
  BLOCK_DELETE,
  BLOCK_SYNC,
  COLLECTION_OPEN,
  COLLECTION_CLOSE,
  COLLECTION_CREATE,
  COLLECTION_DELETE
};

typedef tpie_stats<TPIE_STATS_COLLECTION_COUNT> tpie_stats_collection;

#endif //_TPIE_STATS_COLL_H
