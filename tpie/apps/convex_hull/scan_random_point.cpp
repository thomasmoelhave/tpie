// Copyright (c) 1994 Darren Vengroff
//
// File: scan_random_point.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/19/94
//

static char scan_random_point_id[] = "$Id: scan_random_point.cpp,v 1.4 1999-02-03 22:15:07 tavi Exp $";

// This is just to avoid an error message since the string above is never
// referenced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___scan_random_point_id_compiler_fooler {
    char *pc;
    ___scan_random_point_id_compiler_fooler *next;
} the___scan_random_point_id_compiler_fooler = {
    scan_random_point_id,
    &the___scan_random_point_id_compiler_fooler
};

// Get information on the configuration to test.
#include "app_config.h"

// Define it all.
#include <ami.h>

#include "scan_random_point.h"

scan_random_point::scan_random_point(unsigned int count, int seed) :
max(count), remaining(count)
{
    LOG_INFO("scan_random_point seed = ");
    LOG_INFO(seed);
    LOG_INFO('\n');

    srandom(seed);
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
        out1->x = random() & 0xFFFF;
        out1->y = random() & 0xFFFF;
        } while (((out1->x - 0x7FFF) * (out1->x - 0x7FFF) +
                  (out1->y - 0x7FFF) * (out1->y - 0x7FFF)) > 0x7000 * 0x7000);
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};
