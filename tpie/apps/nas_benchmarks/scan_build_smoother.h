// Copyright (c) 1995 Darren Erik Vengroff
//
// File: scan_build_smoother.h
// Author: Darren Erik Vengroff <dev@cs.duke.edu>
// Created: 3/29/95
//
// $Id: scan_build_smoother.h,v 1.1 1995-04-03 13:17:04 dev Exp $
//
#ifndef _SCAN_BUILD_SMOOTHER_H
#define _SCAN_BUILD_SMOOTHER_H

#include <ami_sparse_matrix.h>

#define SSCI0 12  // = 0.12
#define SSCI1 06  // + 6 * 0.06 = 0.48
#define SSCI2 03  // + 12 * 0.03 = 0.84
#define SSCI3 02  // + 8 * 0.02 = 1.00 

#if ((SSCI0 + 6 * SSCI1 + 12 * SSCI2 + 8 * SSCI3) != 100)
#error Coefficients do not add up.
#endif

#define SSC0 (double(SSCI0)/100.0)
#define SSC1 (double(SSCI1)/100.0)
#define SSC2 (double(SSCI2)/100.0)
#define SSC3 (double(SSCI3)/100.0)

class scan_build_smoother : public AMI_scan_object
{
private:
    static double coeffs[27];
    unsigned int n;
    unsigned int x,y,z;
    int ii,jj,kk;
public:
    scan_build_smoother(unsigned int dim);
    virtual ~scan_build_smoother();
    AMI_err initialize();
    AMI_err operate(AMI_sm_elem<double> *out, AMI_SCAN_FLAG *sf);
};


#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_SBS					\
template AMI_err AMI_scan(scan_build_smoother *,			\
                          AMI_STREAM< AMI_sm_elem<double> > *);

#endif // NO_IMPLICIT_TEMPLATES

#endif // _SCAN_BUILD_SMOOTHER_H 
