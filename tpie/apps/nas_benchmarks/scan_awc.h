// Copyright (c) 1995 Darren Vengroff
//
// File: scan_awc.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/23/95
//
// $Id: scan_awc.h,v 1.3 1999-02-03 22:02:30 tavi Exp $
//
#ifndef _SCAN_AWC_H
#define _SCAN_AWC_H

template<class T>
class scan_add_with_coefficient : AMI_scan_object {
private:
    T c;
public:
    scan_add_with_coefficient() : c(1.0) {};
    virtual ~scan_add_with_coefficient() {};
    void set_coefficient(T co)
    {
        c = co;
    };
    AMI_err initialize(void)
    {
        return AMI_ERROR_NO_ERROR;
    };
    inline AMI_err operate(const T &in1, const T &in2, AMI_SCAN_FLAG *sfin,
                           T *out, AMI_SCAN_FLAG *sfout)
    {
        if ((*sfout = *sfin)) {
            *out = in1 + c * in2;
            return AMI_SCAN_CONTINUE;
        } else {
            return AMI_SCAN_DONE;
        }
    };
};

#endif // _SCAN_AWC_H 
