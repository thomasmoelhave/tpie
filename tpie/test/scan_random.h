// Copyright (c) 1994 Darren Erik Vengroff
//
// File: scan_random.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/6/94
//
// $Id: scan_random.h,v 1.5 1995-06-30 21:09:30 darrenv Exp $
//
#ifndef _SCAN_RANDOM_H
#define _SCAN_RANDOM_H

#if 0
extern "C" void srandom(unsigned int);
// Linux defiens this random as a macro.
#ifndef random
extern "C" long int random(void);
#endif
#endif

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
template AMI_err AMI_scan(scan_random *, AMI_STREAM<int> *);

#endif

#endif // _SCAN_RANDOM_H 
