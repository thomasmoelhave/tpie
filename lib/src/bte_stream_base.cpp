//
// File: bte_stream_base.cpp
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
//         (using some code by Darren Erik Vengroff)
// Created: 01/08/02
//

#include <versions.h>
VERSION(bte_stream_base_cpp,"$Id: bte_stream_base.cpp,v 1.1 2002-01-14 17:18:45 tavi Exp $");

#include <sys/time.h>
#include <sys/resource.h>

#include <config.h>
//#include "lib_config.h"
#include <bte_stream_base.h>

static unsigned long get_remaining_streams() {
  struct rlimit limits;
  if(getrlimit(RLIMIT_NOFILE,&limits) == -1)   {
    // This shouldn't happen, but just in case. Set a conservative value.
    limits.rlim_cur = 255;
  }
  return limits.rlim_cur;
}

tpie_stats_stream BTE_stream_base_generic::gstats_;

int BTE_stream_base_generic::remaining_streams = get_remaining_streams();

