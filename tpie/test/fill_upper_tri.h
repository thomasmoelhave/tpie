// Copyright (c) 1994 Darren Vengroff
//
// File: fill_upper_tri.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/12/94
//
// $Id: fill_upper_tri.h,v 1.5 1999-02-03 21:47:31 tavi Exp $
//
#ifndef _FILL_UPPER_TRI_H
#define _FILL_UPPER_TRI_H

#include <ami_matrix_fill.h>

template<class T>
class fill_upper_tri : public AMI_matrix_filler<T> {
private:
    T val;
public:
    fill_upper_tri(T t) : val(t) {};
    virtual ~fill_upper_tri() {};
    AMI_err initialize(unsigned int /*rows*/, unsigned int /*cols*/)
    {
        return AMI_ERROR_NO_ERROR;
    };
    T element(unsigned int row, unsigned int col)
    {
        return (row <= col) ? val : (T)0;
    };
};

#endif // _FILL_UPPER_TRI_H 
