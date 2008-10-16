// Copyright (c) 1994 Darren Erik Vengroff
//
// File: scan_random.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/6/94
//
// A scan management object to write a stream of random integers.
//

// Get information on the configuration to test.
#include "app_config.h"

#include "scan_random.h"

scan_random::scan_random(TPIE_OS_OFFSET count, int seed) :
    m_max(count), m_remaining(count) {

    TP_LOG_APP_DEBUG("scan_random seed = ");
    TP_LOG_APP_DEBUG(static_cast<TPIE_OS_LONGLONG>(seed));
    TP_LOG_APP_DEBUG('\n');

    TPIE_OS_SRANDOM(seed);
}

scan_random::~scan_random(void) {

    //  No code in this destructor.
}


ami::err scan_random::initialize(void) {
    m_remaining = m_max;

    return ami::NO_ERROR;
};

ami::err scan_random::operate(int *out1, ami::SCAN_FLAG *sf) {
    if ((*sf = (m_remaining-- != 0))) {
        *out1 = TPIE_OS_RANDOM();
        return ami::SCAN_CONTINUE;
    } 
    else {
        return ami::SCAN_DONE;
    }
};

