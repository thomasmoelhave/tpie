// Copyright (c) 1994 Darren Vengroff
//
// File: scan_random_point.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/19/94
//
// $Id: scan_random_point.h,v 1.2 1995-06-20 20:15:38 darrenv Exp $
//
#ifndef _SCAN_RANDOM_POINT_H
#define _SCAN_RANDOM_POINT_H

#include "point.h"

extern "C" int srandom(int);
extern "C" int random(void);

// A scan object to generate random integers.
class scan_random_point : AMI_scan_object {
private:
    unsigned int max, remaining;
public:
    scan_random_point(unsigned int count = 1000, int seed = 17);
    virtual ~scan_random_point(void);
    AMI_err initialize(void);
    AMI_err operate(point<int> *out1, AMI_SCAN_FLAG *sf);
};

#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_SCAN_RANDOM_POINT				\
template AMI_err AMI_scan(scan_random_point *,				\
                          AMI_STREAM< point<int> > *);

#endif

#endif // _SCAN_RANDOM_POINT_H 
