// Copyright (c) 1995 Darren Vengroff
//
// File: scan_value.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/23/95
//
// $Id: scan_value.h,v 1.3 1999-02-03 22:17:30 tavi Exp $
//
#ifndef _SCAN_VALUE_H
#define _SCAN_VALUE_H

template<class T>
class scan_value : AMI_scan_object {
private:
    unsigned int n;
    unsigned int ii;
    T t;
public:
    scan_value(T value, unsigned int count = 1000) : n(count), t(value)
    {
    };
    AMI_err initialize(void)
    {
        ii = n;
        return AMI_ERROR_NO_ERROR;
    };
    inline AMI_err operate(T *out, AMI_SCAN_FLAG *sf)
    {
        if ((*sf = ii--)) {
            *out = t;
            return AMI_SCAN_CONTINUE;
        } else {
            return AMI_SCAN_DONE;
        }
    };
};

#endif // _SCAN_VALUE_H 
