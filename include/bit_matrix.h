// Copyright (c) 1994 Darren Vengroff
//
// File: bit_matrix.h
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/4/94
//
// $Id: bit_matrix.h,v 1.3 1995-06-30 21:08:51 darrenv Exp $
//
#ifndef _BIT_MATRIX_H
#define _BIT_MATRIX_H

#include <bit.h>
#include <matrix.h>

#include <sys/types.h>

#ifdef NO_IMPLICIT_TEMPLATES

#define TEMPLATE_INSTANTIATE_BIT_MATRIX			\
TEMPLATE_INSTANTIATE_MATRIX(bit)

#endif

// typedef matrix<bit> bit_matrix_0;

class bit_matrix : public matrix<bit> {
private:
    bit_matrix::bit_matrix(const matrix<bit> &mb);
public:
    bit_matrix(unsigned int rows, unsigned int cols);
    virtual ~bit_matrix(void);

    bit_matrix operator=(const bit_matrix &rhs);
    
    // We can assign from an offset, which is typically a source
    // address for a BMMC permutation.
    bit_matrix &operator=(const off_t &rhs);

    operator off_t(void);

    friend bit_matrix operator+(const bit_matrix &op1, const bit_matrix &op2);
    friend bit_matrix operator*(const bit_matrix &op1, const bit_matrix &op2);
};

bit_matrix operator+(const bit_matrix &op1, const bit_matrix &op2);
bit_matrix operator*(const bit_matrix &op1, const bit_matrix &op2);

ostream &operator<<(ostream &s, const bit_matrix &bm);


#ifdef NO_IMPLICIT_TEMPLATES
#define TEMPLATE_INSTANTIATE_BIT_MATRIX					\
TEMPLATE_INSTANTIATE_MATRIX(bit)

#endif

#endif // _BIT_MATRIX_H 
