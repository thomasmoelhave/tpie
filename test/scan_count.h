// Copyright (c) 1994 Darren Erik Vengroff
//
// File: scan_count.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/6/94
//
// $Id: scan_count.h,v 1.4 2004-08-12 15:15:11 jan Exp $
//
#ifndef _SCAN_COUNT_H
#define _SCAN_COUNT_H

#include <portability.h>
#include <scan.h>

class scan_count : AMI_scan_object {
private:
    TPIE_OS_OFFSET maximum;
public:
    TPIE_OS_OFFSET ii;
    TPIE_OS_OFFSET called;

    scan_count(TPIE_OS_OFFSET max = 1000);
    AMI_err initialize(void);
    AMI_err operate(TPIE_OS_OFFSET *out1, AMI_SCAN_FLAG *sf);
};

#endif // _SCAN_COUNT_H 
