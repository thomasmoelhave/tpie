// Copyright (c) 1994 Darren Vengroff
//
// File: test_bit_matrix.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/4/94
//

#include <versions.h>
VERSION(test_bit_matrix_cpp,"$Id: test_bit_matrix.cpp,v 1.6 2003-04-20 23:51:40 tavi Exp $");

#include <iostream>
#include <bit_matrix.h>

using std::cout;

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
