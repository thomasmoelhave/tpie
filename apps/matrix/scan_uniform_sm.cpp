// Copyright (c) 1995 Darren Vengroff
//
// File: scan_uniform_sm.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/6/95
//


// Get information on the configuration to test.
#include "app_config.h"

#include "scan_uniform_sm.h"

using namespace tpie;

apps::scan_uniform_sm::scan_uniform_sm(TPIE_OS_OFFSET rows, TPIE_OS_OFFSET cols,
				       double density, int seed) :
    r(0), c(0), rmax(rows), cmax(cols), d(density)
{
    TP_LOG_APP_DEBUG("scan_uniform_sm seed = ");
    TP_LOG_APP_DEBUG(seed);
    TP_LOG_APP_DEBUG('\n');
    
    TPIE_OS_SRANDOM(seed);
}

apps::scan_uniform_sm::~scan_uniform_sm(void) {
}

ami::err apps::scan_uniform_sm::initialize(void) {
    r = rmax - 1;
    c = cmax - 1;
    return ami::NO_ERROR;
}

ami::err apps::scan_uniform_sm::operate(apps::sm_elem<double> *out, ami::SCAN_FLAG *sf)
{
    double dr = double(TPIE_OS_RANDOM() & 0xFFF) / double(0x1000);
    
    if ((*sf = (dr < d))) {
        out->er = r;
        out->ec = c;
        out->val = 1.0;
    } 
    
    if (!(r--)) {
        r = rmax - 1;
        if (!(c--)) {
            return ami::SCAN_DONE;
        }
    }

    return ami::SCAN_CONTINUE;
}
