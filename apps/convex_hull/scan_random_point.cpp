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

VERSION(scan_random_point_cpp,"$Id: scan_random_point.cpp,v 1.7 2003-09-12 01:36:18 tavi Exp $");

#include "scan_random_point.h"

scan_random_point::scan_random_point(unsigned int count, int seed) :
max(count), remaining(count)
{
    LOG_APP_DEBUG("scan_random_point seed = ");
    LOG_APP_DEBUG(seed);
    LOG_APP_DEBUG('\n');

    TPIE_OS_SRANDOM(seed);
}

scan_random_point::~scan_random_point(void)
{
}


AMI_err scan_random_point::initialize(void)
{
    remaining = max;

    return AMI_ERROR_NO_ERROR;
};

AMI_err scan_random_point::operate(point<int> *out1, AMI_SCAN_FLAG *sf)
{
    if ((*sf = remaining--)) {
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
