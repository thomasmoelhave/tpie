// Copyright (c) 1995 Darren Vengroff
//
// File: scan_value.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/23/95
//
// $Id: scan_value.h,v 1.4 2004-08-12 12:37:04 jan Exp $
//
#ifndef _SCAN_VALUE_H
#define _SCAN_VALUE_H

#include <portability.h>

template<class T>
class scan_value : AMI_scan_object {
private:
    TPIE_OS_OFFSET n;
    TPIE_OS_OFFSET ii;
    T t;
public:
    scan_value(T value, TPIE_OS_OFFSET count = 1000) : n(count), t(value)
    {
    };
    AMI_err initialize(void)
    {
        ii = n;
        return AMI_ERROR_NO_ERROR;
    };
    inline AMI_err operate(T *out, AMI_SCAN_FLAG *sf)
    {
        if ((*sf = (ii-- > 0))) {
            *out = t;
            return AMI_SCAN_CONTINUE;
        } else {
            return AMI_SCAN_DONE;
        }
    };
};

#endif // _SCAN_VALUE_H 
