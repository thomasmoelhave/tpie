// Copyright (c) 1994 Darren Vengroff
//
// File: scan_list.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 10/27/94
//
// $Id: scan_list.h,v 1.1 1994-10-31 21:36:27 darrenv Exp $
//
// A scan management object that produces a linked list in order.
//
#ifndef _SCAN_LIST_H
#define _SCAN_LIST_H

#include "list_edge.h"

class scan_list : AMI_scan_object {
private:
    unsigned long int maximum;
public:
    unsigned long int last_to;
    unsigned long int called;

    scan_list(unsigned long int max = 1000);
    AMI_err initialize(void);
    AMI_err operate(edge *out1, AMI_SCAN_FLAG *sf);
};

#endif // _SCAN_LIST_H 
