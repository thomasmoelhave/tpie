// Copyright (C) 2001 Octavian Procopiuc
//
// File:    tpie_stats_coll.h
// Authors: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: tpie_stats_coll.h,v 1.2 2001-12-29 05:17:13 tavi Exp $
//
// Statistics for block collections.

#ifndef _TPIE_STATS_COLL_H
#define _TPIE_STATS_COLL_H

#include <tpie_stats.h>

#define TPIE_STATS_COLL_COUNT 5
enum TPIE_STATS_COLL {
  BLOCK_GET = 0,
  BLOCK_PUT,
  BLOCK_NEW,
  BLOCK_DELETE,
  BLOCK_SYNC
};

typedef tpie_stats<TPIE_STATS_COLL_COUNT> tpie_stats_coll;

#endif //_TPIE_STATS_COLL_H
