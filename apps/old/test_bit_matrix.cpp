// Copyright (c) 1994 Darren Vengroff
//
// File: test_bit_matrix.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/4/94
//

static char test_bit_matrix_id[] = "$Id: test_bit_matrix.cpp,v 1.4 1999-05-02 18:51:47 tavi Exp $";

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

int main(int argc, char **argv)
{
    bit_matrix bm(5,5);

    cout << bm << '\n';

    bm[3][3] = 3;

    bm[2][2] = 2;

    bm[0][0] = bm[1][1] = bm[3][3];
    
    cout << bm << '\n';

    bit_matrix bv(16, 1), bbv(16, 1);

    cout << (bv = (off_t)(0xF0F0)) << '\n';

    off_t ot = 1234;

    bv = ot;

    cout << bv << '\n';

    ot = 4321;

    ot = bv;

    cout << ot << "\n\n";

    bbv = bv + bv;

    cout << bbv << '\n';
    
    return 0;
}
