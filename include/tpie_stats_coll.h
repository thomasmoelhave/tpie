// Copyright (C) 2001 Octavian Procopiuc
//
// File:    tpie_stats_coll.h
// Authors: Octavian Procopiuc <tavi@cs.duke.edu>
//
// $Id: tpie_stats_coll.h,v 1.1 2001-12-29 04:44:11 tavi Exp $
//
// Statistics for block collections.

#ifndef _TPIE_STATS_COLL_H
#define _TPIE_STATS_COLL_H

#define TPIE_STATS_COLL_COUNT 5
enum TPIE_STATS_COLL {
  GET = 0,
  PUT,
  NEW,
  DELETE,
  SYNC
};

typedef tpie_stats<TPIE_STATS_COLL_COUNT> tpie_stats_coll;

#endif //_TPIE_STATS_COLL_H
