// Copyright (c) 1995 Darren Vengroff
//
// File: bit_matrix.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/9/95
//

#include <versions.h>
VERSION(bit_matrix_cpp,"$Id: bit_matrix.cpp,v 1.10 2000-01-10 22:27:11 hutchins Exp $");

#include <bit_matrix.h>

bit_matrix::bit_matrix(unsigned int rows, unsigned int cols) :
        matrix<bit>(rows, cols)
{
}

bit_matrix::bit_matrix(matrix<bit> &mb) :
        matrix<bit>(mb)
{
}
    
bit_matrix::~bit_matrix(void)
{
}

bit_matrix bit_matrix::operator=(const bit_matrix &rhs) {
    return this->matrix<bit>::operator=((matrix<bit> &)rhs);
}

bit_matrix & bit_matrix::operator=(const off_t &rhs)
{
    unsigned int rows = this->rows();
    unsigned int ii;

    if (this->cols() != 1) {
#if HANDLE_EXCEPTIONS
        throw matrix_base<bit>::range();
#else
        tp_assert(0, "Range error.");
#endif
    }
    
    for (ii = 0; ii < rows; ii++) {
        (*this)[ii][0] = (long int)(rhs & (1 << ii)) >> ii;
    }
    
    return *this;
}    

bit_matrix::operator off_t(void)
{
    off_t res;

    unsigned int rows = this->rows();
    unsigned int ii;

    if (this->cols() != 1) {
#if HANDLE_EXCEPTIONS
        throw matrix_base<bit>::range();
#else
        tp_assert(0, "Range error.");
#endif
    }

    for (res = 0, ii = 0; ii < rows; ii++) {
        res |= (long int)((*this)[ii][0]) << ii;
    }
    
    return res;
}


bit_matrix operator+(const bit_matrix &op1, const bit_matrix &op2)
{
    matrix<bit> sum = ((matrix<bit> &)op1) + ((matrix<bit> &)op2);

    return sum;
}

bit_matrix operator*(const bit_matrix &op1, const bit_matrix &op2)
{
    matrix<bit> prod = ((matrix<bit> &)op1) * ((matrix<bit> &)op2);

    return prod;
}

ostream &operator<<(ostream &s, bit_matrix &bm)
{
    return s << (matrix<bit> &)bm;
}
