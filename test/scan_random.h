// Copyright (c) 1994 Darren Erik Vengroff
//
// File: scan_random.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/6/94
//
// $Id: scan_random.h,v 1.2 1994-11-02 21:52:25 darrenv Exp $
//
#ifndef _SCAN_RANDOM_H
#define _SCAN_RANDOM_H

extern "C" int srandom(int);
extern "C" int random(void);

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

#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_SCAN_RANDOM		\
template AMI_err AMI_scan(scan_random *, AMI_base_stream<int> *);

#endif

#endif // _SCAN_RANDOM_H 
