//
// File: bte/stream_base.cpp
// Author: Octavian Procopiuc <tavi@cs.duke.edu>
//         (using some code by Darren Erik Vengroff)
// Created: 01/08/02
//

#include "../lib_config.h"

#include <bte/stream_base.h>

static unsigned long get_remaining_streams() {
    TPIE_OS_SET_LIMITS_BODY;
}

tpie_stats_stream tpie::bte::stream_base_generic::gstats_;

int tpie::bte::stream_base_generic::remaining_streams = get_remaining_streams();

