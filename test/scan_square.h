// Copyright (c) 1994 Darren Erik Vengroff
//
// File: scan_square.h
// Author: Darren Erik Vengroff <darrenv@eecs.umich.edu>
// Created: 10/6/94
//
// $Id: scan_square.h,v 1.3 2005-11-17 17:07:41 jan Exp $
//
// A scan object to square numeric types.
//
#ifndef _SCAN_SQUARE_H
#define _SCAN_SQUARE_H

#include <portability.h>

using namespace tpie;

template<class T> class scan_square : ami::scan_object {
public:
    T ii;
    TPIE_OS_OFFSET called;
    scan_square() : ii(), called(0) {};
    ami::err initialize(void);
    ami::err operate(const T &in, 
		     ami::SCAN_FLAG *sfin,
		     T *out, 
		     ami::SCAN_FLAG *sfout);
};

template<class T>
ami::err scan_square<T>::initialize(void) {

    ii = 0;
    called = 0;

    return ami::NO_ERROR;
};

template<class T>
ami::err scan_square<T>::operate(const T &in, 
				 ami::SCAN_FLAG *sfin,
				 T *out, 
				 ami::SCAN_FLAG *sfout) {

    called++;
    
    if ((*sfout = *sfin)) {
        ii = in;
        *out = in * in;

	return ami::SCAN_CONTINUE;

    } 
    else {
        return ami::SCAN_DONE;
    }
};


#endif // _SCAN_SQUARE_H 
