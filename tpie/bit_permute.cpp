// Copyright (c) 1995 Darren Vengroff
//
// File: ami_bit_permute.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/9/95
//

#include <iostream>

#include <versions.h>
VERSION(ami_bit_permute_cpp,"$Id: ami_bit_permute.cpp,v 1.4 2003-04-20 06:44:01 tavi Exp $");

#include <bit_permute.h>

using tpie::ami;

bit_perm_object::bit_perm_object(const bit_matrix &A,
				 const bit_matrix &c) :
    mA(A), mc(c) {
    //  No code in this constructor.
}

bit_perm_object::~bit_perm_object(void) {

    //  No code in this destructor.

}

bit_matrix bit_perm_object::A(void) {
    return mA;
}

bit_matrix bit_perm_object::c(void) {
    return mc;
}


