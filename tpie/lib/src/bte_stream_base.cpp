//
// File: bte_stream_base.cpp
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
//         (using some code by Darren Erik Vengroff)
// Created: 01/08/02
//

#include <versions.h>
VERSION(bte_stream_base_cpp,"$Id: bte_stream_base.cpp,v 1.2 2003-04-17 20:57:33 jan Exp $");

#include <config.h>
//#include "lib_config.h"
#include <bte_stream_base.h>

static unsigned long get_remaining_streams() {
	TPIE_OS_SET_LIMITS_BODY;
}

tpie_stats_stream BTE_stream_base_generic::gstats_;

int BTE_stream_base_generic::remaining_streams = get_remaining_streams();

