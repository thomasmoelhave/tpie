// Copyright (c) 1994 Darren Erik Vengroff
//
// File: scan_random.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/6/94
//
// $Id: scan_random.h,v 1.8 2003-04-21 02:47:47 tavi Exp $
//
#ifndef _SCAN_RANDOM_H
#define _SCAN_RANDOM_H

#include <ami_scan.h>

// A scan object to generate random integers.
class scan_random : AMI_scan_object {
private:
    unsigned int max, remaining;
public:
    scan_random(unsigned int count = 1000, int seed = 17);
    virtual ~scan_random(void);
    AMI_err initialize(void);
    AMI_err operate(int *out1, AMI_SCAN_FLAG *sf);
};

#endif // _SCAN_RANDOM_H 
