// Copyright (c) 1995 Darren Vengroff
//
// File: scan_uniform_sm.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/6/95
//

static char scan_uniform_sm_id[] = "$Id: scan_uniform_sm.cpp,v 1.2 1997-05-20 22:12:09 vengroff Exp $";

// This is just to avoid an error message since the string above is never
// referenced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___scan_uniform_sm_id_compiler_fooler {
    char *pc;
    ___scan_uniform_sm_id_compiler_fooler *next;
} the___scan_uniform_sm_id_compiler_fooler = {
    scan_uniform_sm_id,
    &the___scan_uniform_sm_id_compiler_fooler
};



// Get information on the configuration to test.
#include "app_config.h"

// Define it all.
#include <ami.h>

#include "scan_uniform_sm.h"

scan_uniform_sm::scan_uniform_sm(unsigned int rows, unsigned int cols,
                                 double density, int seed) :
        rmax(rows), cmax(cols), d(density)
{
    LOG_INFO("scan_uniform_sm seed = ");
    LOG_INFO(seed);
    LOG_INFO('\n');

    srandom(seed);
}

scan_uniform_sm::~scan_uniform_sm(void)
{
}

AMI_err scan_uniform_sm::initialize(void)
{
    r = rmax - 1;
    c = cmax - 1;
    return AMI_ERROR_NO_ERROR;
}

AMI_err scan_uniform_sm::operate(AMI_sm_elem<double> *out, AMI_SCAN_FLAG *sf)
{
    double dr = double(random() & 0xFFF) / double(0x1000);

    if ((*sf = (dr < d))) {
        out->er = r;
        out->ec = c;
        out->val = 1.0;
    } 
        
    if (!(r--)) {
        r = rmax - 1;
        if (!(c--)) {
            return AMI_SCAN_DONE;
        }
    }

    return AMI_SCAN_CONTINUE;
}
