// Copyright (c) 1995 Darren Vengroff
//
// File: scan_gauss.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/22/95
//
// $Id: scan_gauss.h,v 1.1 1995-04-03 13:17:47 dev Exp $
//
#ifndef _SCAN_GAUSS_H
#define _SCAN_GAUSS_H

class scan_gauss : public AMI_scan_object {
private:
    // Are we about to read the i'th input where i is even?
    bool even;
    // The input values from the last two times.
    double xj,yj;
public:
    // The sums of the x and y values output so far.
    double sumx, sumy;

    // Annulus counts.
    unsigned int annulus[10];

    scan_gauss();
    ~scan_gauss();
    AMI_err initialize(void);
    AMI_err operate(const double &r, AMI_SCAN_FLAG *sfin,
                    double *x, double *y, AMI_SCAN_FLAG *sfout);
};


#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_SCAN_GAUSS					\
template AMI_err AMI_scan(AMI_STREAM<double> *, scan_gauss *,		\
                          AMI_STREAM<double> *, AMI_STREAM<double> *);


#endif


#endif // _SCAN_GAUSS_H 
