// Copyright (c) 1994 Darren Vengroff
//
// File: bit_matrix.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/4/94
//
// $Id: bit_matrix.h,v 1.1 1994-12-16 21:48:02 darrenv Exp $
//
#ifndef _BIT_MATRIX_H
#define _BIT_MATRIX_H

#include <bit.h>
#include <matrix.h>


#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_BIT_MATRIX			\
TEMPLATE_INSTANTIATE_MATRIX(bit)

#endif

typedef matrix<bit> bit_matrix;

#endif // _BIT_MATRIX_H 
