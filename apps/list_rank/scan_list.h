// Copyright (c) 1994 Darren Vengroff
//
// File: scan_list.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 10/27/94
//
// $Id: scan_list.h,v 1.2 2004-08-12 12:36:45 jan Exp $
//
// A scan management object that produces a linked list in order.
//
#ifndef _SCAN_LIST_H
#define _SCAN_LIST_H

#include "list_edge.h"

class scan_list : AMI_scan_object {
private:
    TPIE_OS_OFFSET maximum;
public:
    TPIE_OS_OFFSET last_to;
    TPIE_OS_OFFSET called;

    scan_list(TPIE_OS_OFFSET max = 1000);
    AMI_err initialize(void);
    AMI_err operate(edge *out1, AMI_SCAN_FLAG *sf);
};

#endif // _SCAN_LIST_H 
