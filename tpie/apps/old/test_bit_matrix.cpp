// Copyright (c) 1994 Darren Vengroff
//
// File: test_bit_matrix.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/4/94
//

#include <portability.h>

#include <versions.h>
VERSION(test_bit_matrix_cpp,"$Id: test_bit_matrix.cpp,v 1.7 2003-09-12 01:34:41 tavi Exp $");

#include <bit_matrix.h>


int main(int argc, char **argv)
{
    bit_matrix bm(5,5);
    cout << bm << endl;

    bm[3][3] = 3;
    bm[2][2] = 2;
    bm[0][0] = bm[1][1] = bm[3][3];
    cout << bm << endl;

    bit_matrix bv(16, 1), bbv(16, 1);
    cout << (bv = (off_t)(0xF0F0)) << endl;
    off_t ot = 1234;
    bv = ot;
    cout << bv << endl;

    ot = 4321;
    ot = bv;
    cout << ot << endl << endl;

    bbv = bv + bv;
    cout << bbv << endl;
    
    return 0;
}
