// Copyright (c) 1995 Darren Erik Vengroff
//
// File: scan_build_smoother.cpp
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 3/29/95
//

static char scan_build_smoother_id[] = "$Id: scan_build_smoother.cpp,v 1.2 2004-08-12 12:37:04 jan Exp $";


// Get information on the configuration to test.
#include "app_config.h"

// Define it all.
#include <ami.h>


#include "scan_build_smoother.h"


double scan_build_smoother::coeffs[27] = {
    SSC3, SSC2, SSC3,    SSC2, SSC1, SSC2,    SSC3, SSC2, SSC3,
    SSC2, SSC1, SSC2,    SSC1, SSC0, SSC1,    SSC2, SSC1, SSC2,
    SSC3, SSC2, SSC3,    SSC2, SSC1, SSC2,    SSC3, SSC2, SSC3
};


scan_build_smoother::scan_build_smoother(TPIE_OS_OFFSET dim) :
        n(dim)
{
}
                                        

scan_build_smoother::~scan_build_smoother()
{    
}


AMI_err scan_build_smoother::initialize()
{
    x = y = z = 0;
    ii = jj = kk = -1;

    return AMI_ERROR_NO_ERROR;
}


AMI_err scan_build_smoother::operate(AMI_sm_elem<double> *out,
                                     AMI_SCAN_FLAG *sf)
{
    *sf = 1;
    out->er = x + n*y + n*n*z;
    out->ec = ((x+ii+n)%n) + n*((y+jj+n)%n) + n*n*((z+jj+n)%n);
    out->val = coeffs[(ii+1) + 3*(jj+1) + 9*(kk+1)];
    if ((ii = ((ii+2) % 3) - 1) == -1) {
        if ((jj = ((jj+2) % 3) - 1) == -1) {
            if ((kk = ((kk+2) % 3) - 1) == -1) {
                if (!(x = (x+1) % n)) {
                    if (!(y = (y+1) % n)) {
                        if (!(z = (z+1) % n)) {
                            return AMI_SCAN_DONE;
                        }
                    }
                }
            }
        }
    }
    return AMI_SCAN_CONTINUE;
}
