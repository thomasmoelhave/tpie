// Copyright (c) 1994 Darren Erik Vengroff
//
// File: scan_count.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/6/94
//
// $Id: scan_count.h,v 1.1 1994-10-07 15:47:12 darrenv Exp $
//
#ifndef _SCAN_COUNT_H
#define _SCAN_COUNT_H

class scan_count : AMI_scan_object {
private:
    int maximum;
public:
    int ii;
    unsigned long int called;

    scan_count(int max = 1000) : maximum(max), ii(0) {};
    AMI_err initialize(void);
    AMI_err operate(int *out1, AMI_SCAN_FLAG *sf);
};

#endif // _SCAN_COUNT_H 
