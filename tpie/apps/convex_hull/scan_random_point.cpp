// Copyright (c) 1994 Darren Vengroff
//
// File: scan_random_point.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/19/94
//

// Get information on the configuration to test.
#include "app_config.h"

// Define it all.
#include <ami.h>

VERSION(scan_random_point_cpp,"$Id: scan_random_point.cpp,v 1.9 2004-08-12 12:36:24 jan Exp $");

#include "scan_random_point.h"

scan_random_point::scan_random_point(TPIE_OS_OFFSET count, int seed) 
{
    this->max = count;
    this->remaining = count;
    TP_LOG_APP_DEBUG("scan_random_point seed = ");
    TP_LOG_APP_DEBUG(seed);
    TP_LOG_APP_DEBUG('\n');

    TPIE_OS_SRANDOM(seed);
}

scan_random_point::~scan_random_point(void)
{
}


AMI_err scan_random_point::initialize(void)
{
    this->remaining = this->max;

    return AMI_ERROR_NO_ERROR;
};

AMI_err scan_random_point::operate(point<int> *out1, AMI_SCAN_FLAG *sf)
{
    if ((*sf = (remaining-- > 0))) {
        do {
        out1->x = TPIE_OS_RANDOM() & 0xFFFF;
        out1->y = TPIE_OS_RANDOM() & 0xFFFF;
        } while (((out1->x - 0x7FFF) * (out1->x - 0x7FFF) +
                  (out1->y - 0x7FFF) * (out1->y - 0x7FFF)) > 0x7000 * 0x7000);
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};
