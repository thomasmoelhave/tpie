// Copyright (c) 1995 Darren Vengroff
//
// File: scan_gauss.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/22/95
//

static char scan_gauss_id[] = "$Id: scan_gauss.cpp,v 1.1 1995-04-03 13:17:39 dev Exp $";

// This is just to avoid an error message since the string above is never
// referenced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___scan_gauss_id_compiler_fooler {
    char *pc;
    ___scan_gauss_id_compiler_fooler *next;
} the___scan_gauss_id_compiler_fooler = {
    scan_gauss_id,
    &the___scan_gauss_id_compiler_fooler
};


// Get information on the configuration to test.
#include "app_config.h"

// Define it all.
#include <ami.h>


#include "scan_gauss.h"

scan_gauss::scan_gauss()
{
}

scan_gauss::~scan_gauss()
{
}

AMI_err scan_gauss::initialize(void)
{
    even = false;
    sumx = sumy = 0.0;
    for (unsigned int ii = 10; ii--; ) {
        annulus[ii] = 0;
    }    
    return AMI_ERROR_NO_ERROR;
}


AMI_err scan_gauss::operate(const double &r, AMI_SCAN_FLAG *sfin,
                            double *x, double *y, AMI_SCAN_FLAG *sfout)
{
    register double tj,tj2;
    register double outx, outy;
    
    if (*sfin) {
        if (even) {
            yj = 2*r - 1;
            if ((tj = xj*xj+yj*yj) <= 1.0) {
                // In the unit circle.
                tj2 = sqrt(-2.0*log(tj)/tj);
                *x = outx = xj*tj2;
                *y = outy = yj*tj2;
                sfout[0] = sfout[1] = 1;

                // Keep a running total.
                sumx += outx;
                sumy += outy;

                // Count values in succesive square annuli.  Moving
                // outward probably beats binary seach since they are
                // concentrated near the origin.
                {
                    outx = fabs(outx);
                    outy = fabs(outy);
                    register double m = (outx > outy) ? outx : outy;  
                    for (unsigned int ii = 0; ii++ < 9; ) {
                        if (m < ii) {                   
                            annulus[ii-1]++;
                            break;
                        }
                    }
                }
               
            } else {
                sfout[0] = sfout[1] = 0;
            }
        } else {
            sfout[0] = sfout[1] = 0;
            xj = 2*r - 1;
        }
        even = !even;
        return AMI_SCAN_CONTINUE;
    } else {
        sfout[0] = sfout[1] = 0;
        return AMI_SCAN_DONE;
    }
}

