// Copyright (c) 1994 Darren Erik Vengroff
//
// File: scan_random.cpp
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/6/94
//
// A scan management object to write a stream of random integers.
//

static char scan_random_id[] = "$Id: scan_random.cpp,v 1.4 1999-10-19 15:05:47 tavi Exp $";

// Get information on the configuration to test.
#include "app_config.h"

// Define it all.
#include <ami.h>

#include "scan_random.h"

scan_random::scan_random(unsigned int count, int seed) :
max(count), remaining(count)
{
    LOG_DEBUG_INFO("scan_random seed = ");
    LOG_DEBUG_INFO(seed);
    LOG_DEBUG_INFO('\n');

    srandom(seed);
}

scan_random::~scan_random(void)
{
}


AMI_err scan_random::initialize(void)
{
    remaining = max;

    return AMI_ERROR_NO_ERROR;
};

inline AMI_err scan_random::operate(int *out1, AMI_SCAN_FLAG *sf)
{
    if ((*sf = (remaining-- != 0))) {
        *out1 = random();
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};

