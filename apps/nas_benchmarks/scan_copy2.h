// Copyright (c) 1995 Darren Vengroff
//
// File: scan_copy2.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/23/95
//
// $Id: scan_copy2.h,v 1.3 1999-02-03 22:04:12 tavi Exp $
//
#ifndef _SCAN_COPY2_H
#define _SCAN_COPY2_H

template<class T>
class scan_copy2 : AMI_scan_object {
public:
    scan_copy2() {};
    virtual ~scan_copy2() {};
    AMI_err initialize(void)
    {
        return AMI_ERROR_NO_ERROR;
    };
    inline AMI_err operate(const T &in, AMI_SCAN_FLAG *sfin,
                           T *out1, T *out2, AMI_SCAN_FLAG *sfout)
    {
        if ((sfout[0] = sfout[1] = *sfin)) {
            *out1 = *out2 = in;
            return AMI_SCAN_CONTINUE;
        } else {
            return AMI_SCAN_DONE;
        }
    };
};

#endif // _SCAN_COPY2_H 
