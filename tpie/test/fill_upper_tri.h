// Copyright (c) 1994 Darren Vengroff
//
// File: fill_upper_tri.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 12/12/94
//
// $Id: fill_upper_tri.h,v 1.1 1995-03-07 15:07:27 darrenv Exp $
//
#ifndef _FILL_UPPER_TRI_H
#define _FILL_UPPER_TRI_H

#include <ami_matrix_fill.h>

template<class T>
class fill_upper_tri : public AMI_matrix_filler<T> {
    AMI_err initialize(unsigned int rows, unsigned int cols)
    {
        return AMI_ERROR_NO_ERROR;
    };
    T element(unsigned int row, unsigned int col)
    {
        return (row <= col) ? (T)1 : (T)0;
    };
};


#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_FILL_UPPER_TRI(T)				\
TEMPLATE_INSTANTIATE_MATRIX_FILL(T)					\
template class fill_upper_tri<T>;

#endif

#endif // _FILL_UPPER_TRI_H 
