// Copyright (c) 1994 Darren Vengroff
//
// File: test_bit_matrix.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/4/94
//

static char test_bit_matrix_id[] = "$Id: test_bit_matrix.cpp,v 1.1 1994-12-16 21:26:04 darrenv Exp $";

// This is just to avoid an error message since the string above is never
// referenced.  Note that a self referential structure must be defined to
// avoid passing the problem further.
static struct ___test_bit_matrix_id_compiler_fooler {
    char *pc;
    ___test_bit_matrix_id_compiler_fooler *next;
} the___test_bit_matrix_id_compiler_fooler = {
    test_bit_matrix_id,
    &the___test_bit_matrix_id_compiler_fooler
};


#include <iostream.h>

#include <bit_matrix.h>

TEMPLATE_INSTANTIATE_BIT_MATRIX

int main(int argc, char **argv)
{
    bit_matrix bm(5,5);

    cout << bm << '\n';

    bm[3][3] = 3;

    bm[2][2] = 2;

    bm[0][0] = bm[1][1] = bm[3][3];
    
    cout << bm << '\n';
    
    return 0;
}
