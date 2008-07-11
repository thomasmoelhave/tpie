// Copyright (c) 1994 Darren Vengroff
//
// File: test_matrix.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 11/4/94
//

#include <portability.h>

#include <versions.h>
VERSION(test_matrix_cpp,"$Id: test_matrix.cpp,v 1.5 2003-09-11 21:46:22 tavi Exp $");

#include "app_config.h"
#include <matrix.h>


int main(int argc, char **argv)
{
    matrix<int> um(5,5);

    um[0][0] = 1;
    um[0][1] = 2;
    um[1][0] = 3;
    um[1][1] = 4;

    cout << um << endl;
    um += um;
    cout << um << endl;
    um = um * um;
    cout << um << endl;
    matrix<int> copy(um);
    copy += um;
    cout << copy << endl << um << endl;
    um = um + copy;
    cout << um << endl;
    submatrix<int> sm0(um,0,1,3,4);
    cout << sm0 << endl;
    sm0[0][0] = 1;
    sm0[1][1] = 1;
    cout << sm0 << endl;
    cout << um << endl;
    submatrix<int>  sm1(um,0,1,0,1);
    submatrix<int> sm2(um,3,4,3,4);
    sm2 = sm0 + sm1;
    cout << um << endl;    
    um = um * um;
    cout << um << endl;    
    
    // Test row references.
    rowref<int> rr(um,2);
    cout << matrix<int>(rr) << endl;
    cout << um << endl;    
    rr[0] = rr[1] = rr[2] = rr[3] = rr[4] = 17;
    cout << matrix<int>(rr) << endl;
    cout << um << endl;
    
    // Test column references.
    colref<int> cr(um,4);
    cout << matrix<int>(cr) << endl;
    cout << um << endl;    
    cr[0] = cr[1] = cr[2] = cr[3] = cr[4] = -289;
    cout << matrix<int>(cr) << endl;
    cout << um << endl;
    
    matrix<int> umsub(3,3);
    umsub = submatrix<int>(um,1,3,1,3);
    umsub += umsub;
    cout << umsub << endl;
    cout << um << endl;


    // Mutiply the row reference by the column reference in both orders.
    cout << matrix<int>(rr) * matrix<int>(cr) << endl;
    cout << matrix<int>(cr) * matrix<int>(rr) << endl;    

    // Test the copy constructor to make sure it does an elementwise copy.
    matrix<int>cp(um);
    cp[3][3] = 99999;
    cout << cp << endl << um << endl;

    // Test construction from a submatrix.
    matrix<int>csm(submatrix<int>(um,2,3,2,3));
    cout << csm << endl;
    
    // Try with double.
    matrix<double>dm(4,4);
    double &dr = dm[0][0];
    dr = 1.0;
    cout << dm[0][0] << endl;
    cout << dm << endl;

    return 0;
}
