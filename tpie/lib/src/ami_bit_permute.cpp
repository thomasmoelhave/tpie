// Copyright (c) 1995 Darren Vengroff
//
// File: ami_bit_permute.cpp
// Author: Darren Vengroff <darrenv@eecs.umich.edu>
// Created: 1/9/95
//

#include <iostream.h>

#include <versions.h>
VERSION(ami_bit_permute_cpp,"$Id: ami_bit_permute.cpp,v 1.3 2003-04-17 20:38:19 jan Exp $");

#include <ami_bit_permute.h>

AMI_bit_perm_object::AMI_bit_perm_object(const bit_matrix &A,
                                         const bit_matrix &c) :
                                                 mA(A), mc(c)
{
}

AMI_bit_perm_object::~AMI_bit_perm_object(void)
{
}

bit_matrix AMI_bit_perm_object::A(void)
{
    return mA;
}

bit_matrix AMI_bit_perm_object::c(void)
{
    return mc;
}


