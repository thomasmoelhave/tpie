// Copyright (c) 1995 Darren Vengroff
//
// File: scan_pr.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/22/95
//

static char scan_pr_id[] = "$Id: scan_pr.cpp,v 1.1 1995-04-03 13:18:19 dev Exp $";

// This is just to avoid an error message since the string above is never
// referenced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___scan_pr_id_compiler_fooler {
    char *pc;
    ___scan_pr_id_compiler_fooler *next;
} the___scan_pr_id_compiler_fooler = {
    scan_pr_id,
    &the___scan_pr_id_compiler_fooler
};


// Get information on the configuration to test.
#include "app_config.h"

// Define it all.
#include <ami.h>

#include "scan_pr.h"

#include <math.h>

// Constants definied in the NAS EP benchmark.

#define TWO_TO_23 (double(2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2))
#define TWO_TO_46 (TWO_TO_23 * TWO_TO_23)

#define TWO_TO_MINUS_23 (1.0 / TWO_TO_23)
#define TWO_TO_MINUS_46 (1.0 / TWO_TO_46)


// Member fucntions:

scan_nas_psuedo_rand::scan_nas_psuedo_rand(double seed,
                                           unsigned int count,
                                           double a) :
                                                   s(seed),
                                                   max(count)
{
        a1 = floor(TWO_TO_MINUS_23 * a);
        a2 = a - TWO_TO_23 * a1;    
}


scan_nas_psuedo_rand::~scan_nas_psuedo_rand()
{
}

AMI_err scan_nas_psuedo_rand::initialize(void)
{
    x = s;
    remaining = max;

    return AMI_ERROR_NO_ERROR;
}

AMI_err scan_nas_psuedo_rand::operate(double *out, AMI_SCAN_FLAG *sf)
{
    register double b1, b2;
    register double t1, t2, t3, t4, t5;

    if (*sf = remaining--) {
        b1 = floor(TWO_TO_MINUS_23 * x);
        b2 = x - TWO_TO_23 * b1;

        t1 = a1*b2 + a2*b1;
        t2 = floor(TWO_TO_MINUS_23 * t1);
        t3 = t1 - TWO_TO_23 * t2;
        t4 = TWO_TO_23 * t3 + a2*b2;
        t5 = floor(TWO_TO_MINUS_46 * t4);

        x = t4 - TWO_TO_46 * t5;

        *out = x * TWO_TO_MINUS_46;
             
        return AMI_SCAN_CONTINUE;

    } else {
        return AMI_SCAN_DONE;
    }
}

