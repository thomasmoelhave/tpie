// Copyright (c) 1995 Darren Vengroff
//
// File: scan_pr.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/22/95
//
// $Id: scan_pr.h,v 1.1 1995-04-03 13:18:28 dev Exp $
//
// Psuedo-random numbers for the NAS parallel benchmarks.
//
#ifndef _SCAN_PR_H
#define _SCAN_PR_H

#define COUNT (1024*1024)
#define NAS_S 271828183
#define NAS_A (5*5*5*5*5*5*5*5*5*5*5*5*5)

class scan_nas_psuedo_rand : AMI_scan_object {
private:
    // The seed.
    double s;

    // The last value output.
    double x;

    // A cache for the multiplicative factor a.
    double a1, a2;
    
    unsigned int max, remaining;
public:
    scan_nas_psuedo_rand(double seed = NAS_S,
                         unsigned int count = COUNT,
                         double a = NAS_A);
                         
    virtual ~scan_nas_psuedo_rand(void);
    AMI_err initialize(void);
    AMI_err operate(double *out, AMI_SCAN_FLAG *sf);
};



#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_SCAN_PR					\
template AMI_err AMI_scan(scan_nas_psuedo_rand *,			\
                          AMI_STREAM<double> *);


#endif



#endif // _SCAN_PR_H 

