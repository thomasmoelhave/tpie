// Copyright (c) 1995 Darren Vengroff
//
// File: fill_value.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 3/6/95
//
// $Id: fill_value.h,v 1.2 1997-05-20 22:06:25 vengroff Exp $
//
#ifndef _FILL_VALUE_H
#define _FILL_VALUE_H

#include <ami_matrix_fill.h>

template<class T>
class fill_value : public AMI_matrix_filler<T> {
private:
    T value;
public:
    void set_value(const T &v) {
        value = v;
    };
    AMI_err initialize(unsigned int /*rows*/, unsigned int /*cols*/)
    {
        return AMI_ERROR_NO_ERROR;
    };
    T element(unsigned int /*row*/, unsigned int /*col*/)
    {
        return value;
    };
};


#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_FILL_VALUE(T)				\
TEMPLATE_INSTANTIATE_MATRIX_FILL(T)					\
template class fill_value<T>;

#endif


#endif // _FILL_VALUE_H 
