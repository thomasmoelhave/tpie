// Copyright (c) 1995 Darren Vengroff
//
// File: bit_matrix.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/9/95
//

static char bit_matrix_id[] = "$Id: bit_matrix.cpp,v 1.4 1997-05-20 22:04:13 vengroff Exp $";

// This is just to avoid an error message since the string above is never
// referenced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___bit_matrix_id_compiler_fooler {
    char *pc;
    ___bit_matrix_id_compiler_fooler *next;
} the___bit_matrix_id_compiler_fooler = {
    bit_matrix_id,
    &the___bit_matrix_id_compiler_fooler
};



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
    this->matrix<bit>::operator=((matrix<bit> &)rhs);
}

bit_matrix & bit_matrix::operator=(const off_t &rhs)
{
    unsigned int rows = this->rows();
    unsigned int ii;

    if (this.cols() != 1) {
#if HANDLE_EXCEPTIONS
        throw matrix_base<bit>::range();
#else
        tp_assert(0, "Range error.");
#endif
    }
    
    for (ii = 0; ii < rows; ii++) {
        (*this)[ii][0] = (rhs & (1 << ii)) >> ii;
    }
    
    return *this;
}    

bit_matrix::operator off_t(void)
{
    off_t res;

    unsigned int rows = this->rows();
    unsigned int ii;

    if (this.cols() != 1) {
#if HANDLE_EXCEPTIONS
        throw matrix_base<bit>::range();
#else
        tp_assert(0, "Range error.");
#endif
    }

    for (res = 0, ii = 0; ii < rows; ii++) {
        res |= (off_t)((*this)[ii][0]) << ii;
    }
    
    return res;
}


bit_matrix operator+(bit_matrix &op1, bit_matrix &op2)
{
    matrix<bit> sum = ((matrix<bit> &)op1) + ((matrix<bit> &)op2);

    return sum;
}

bit_matrix operator*(bit_matrix &op1, bit_matrix &op2)
{
    matrix<bit> prod = ((matrix<bit> &)op1) * ((matrix<bit> &)op2);

    return prod;
}

ostream &operator<<(ostream &s, bit_matrix &bm)
{
    return s << (matrix<bit> &)bm;
}


#ifdef NO_IMPLICIT_TEMPLATES
TEMPLATE_INSTANTIATE_BIT_MATRIX
#endif
