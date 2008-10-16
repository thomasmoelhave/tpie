// Copyright (c) 1994 Darren Vengroff
//
// File: fill_upper_tri.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/12/94
//
// $Id: fill_upper_tri.h,v 1.7 2005-11-16 17:04:32 jan Exp $
//
#ifndef _FILL_UPPER_TRI_H
#define _FILL_UPPER_TRI_H

#include <portability.h>

#include <matrix_fill.h>

template<class T>
class fill_upper_tri : public AMI_matrix_filler<T> {
private:
    T val;
public:
    fill_upper_tri(T t) : val(t) {};
    virtual ~fill_upper_tri() {};
    AMI_err initialize(TPIE_OS_OFFSET /*rows*/, TPIE_OS_OFFSET /*cols*/)
    {
        return AMI_ERROR_NO_ERROR;
    };
    T element(TPIE_OS_OFFSET row, TPIE_OS_OFFSET col)
    {
        return (row <= col) ? val : 0;
    };
};

#endif // _FILL_UPPER_TRI_H 
