// Copyright (c) 1994 Darren Erik Vengroff
//
// File: scan_square.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/6/94
//
// $Id: scan_square.h,v 1.1 1994-10-07 15:48:17 darrenv Exp $
//
// A scan object to square numeric types.
//
#ifndef _SCAN_SQUARE_H
#define _SCAN_SQUARE_H


template<class T> class scan_square : AMI_scan_object {
public:
    T ii;
    unsigned long int called;
    AMI_err initialize(void);
    AMI_err operate(const T &in, AMI_SCAN_FLAG *sfin,
                    T *out, AMI_SCAN_FLAG *sfout);
};

template<class T>
AMI_err scan_square<T>::initialize(void)
{
    ii = 0;
    called = 0;
    return AMI_ERROR_NO_ERROR;
};

template<class T>
AMI_err scan_square<T>::operate(const T &in, AMI_SCAN_FLAG *sfin,
                                T *out, AMI_SCAN_FLAG *sfout)
{
    called++;
    if (*sfout = *sfin) {
        ii = in;
        *out = in * in;
        return AMI_SCAN_CONTINUE;
    } else {
        return AMI_SCAN_DONE;
    }
};


#endif // _SCAN_SQUARE_H 
